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
#define LOG_TAG "paramctrl_per_frame"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_error_code.h>
//#include <mtkcam/hal/aaa/aaa_hal_if.h>
//#include <mtkcam/hal/aaa/aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "isp_mgr_af_stat.h"
#include <pca_mgr.h>
#include <ispfeature.h>
#include <isp_interpolation/isp_interpolation.h>
#include <ccm_mgr.h>
#include <ggm_mgr.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <lsc/ILscMgr.h>
#include <mtkcam/drv/IHalSensor.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include "isp_tuning_mgr.h"
#include <mtkcam/utils/sys/IFileCache.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

//#include <hwutils/CameraProfile.h>
//#include "vfb_hal_base.h"
//#include <mtkcam/featureio/capturenr.h>

//define log control
#define EN_3A_FLOW_LOG        1
#define EN_3A_SCHEDULE_LOG    2

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSCam::TuningUtils;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_P1(RequestSet_T const RequestSet, MINT32 i4SubsampleIdex)
{
    MBOOL fgRet = MTRUE;
    MINT32 i4FrameID = RequestSet.vNumberSet[0];

    CAM_LOGD_IF(m_bDebugEnable, "[%s] + i4FrameID(%d), i4SubsampleIdex(%d)", __FUNCTION__, i4FrameID, i4SubsampleIdex);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    int r3ALogEnable = atoi(value);
    AaaTimer localTimer("applyToHwAll", m_eSensorDev, (r3ALogEnable & EN_3A_SCHEDULE_LOG));

    MINT32 i4Magic = i4FrameID;
    MINT32 i4NumInRequestSet;
    RequestSet_T RequestTuningSet =  RequestSet;
    AAA_TRACE_DRV(applyToHw_perFrame_All);
    m_pTuning->dequeBuffer(RequestTuningSet.vNumberSet.data());

//if (RequestSet.vNumberSet[0] == 1)
{
    for (MINT32 i4InitNum = 0; i4InitNum < m_i4SubsampleCount; i4InitNum++)
    {
        fgRet = ISP_MGR_AE_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_AWB_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_AF_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4InitNum)
            &&  ISP_MGR_PDO_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4InitNum)
            &&  ISP_MGR_FLK_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, m_bFlkEnable, i4InitNum)
            &&  ISP_MGR_SL2F_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.RawSize, m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4InitNum)
            // ADBS should be after DBS
            &&  ISP_MGR_ADBS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RMM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RMG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            // LCS shoud be after CCM, RPG
            &&  ISP_MGR_LCS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_CPN_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)  //should be after LSC
            &&  ISP_MGR_DCPN_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum) //should be after LSC
            ;
    }
}
    AAA_TRACE_END_DRV;

/*else
{
    fgRet = ISP_MGR_AE_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_AWB_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_AF_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4SubsampleIdex)
        &&  ISP_MGR_FLK_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, m_bFlkEnable, i4SubsampleIdex)
        &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_RMM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.rMapping_Info.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_RMG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4SubsampleIdex)
        ;
}*/


    m_pTuning->enqueBuffer();

    localTimer.End();

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
applyToHw_PerFrame_P2(MINT32 /*flowType*/, const ISP_INFO_T& rIspInfo, void* pRegBuf)
{
    AAA_TRACE_HAL(applyToHw_PerframeP2);
    MBOOL fgRet = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] + pRegBuf(%p)", __FUNCTION__, pRegBuf);

    const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;
    dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);

    if(rIspCamInfo.rMapping_Info.eIspProfile != EIspProfile_Bokeh){
        ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));
    }

    fgRet = MTRUE
        &&  ISP_MGR_SL2G_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_DBS2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_ADBS2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, rIspCamInfo, pReg)
        &&  ISP_MGR_OBC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_BNR2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_LSC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_SL2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspInfo.rIspP2CropInfo, rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        //RNR should be after OBC2 & PGN
        &&  ISP_MGR_RNR_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_UDM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_CCM2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_FLC_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_GGM2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_LCE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_NDG_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_NDG2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_NBC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_HFG_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        //&&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pReg)
        &&  ISP_MGR_RMM2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_RMG2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_CPN2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_DCPN2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        ;

    //factory no need BPCI
    if(m_eOperMode == EOperMpde_Factory){
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 0;
        pReg->DIP_X_CTL_RGB_EN.Bits.LSC2_EN = 0;
        pReg->DIP_X_CTL_DMA_EN.Bits.DEPI_EN = 0;
    }

    // the protection of HW limitations
    ispHWConstraintSet(pReg);   // the protection of HW limitations

/*
    CAM_LOGE("Chooo, P2 Pro: %d, Sen: %d, PDC_EN: %d, BPC_EN: %d, LUT_EN:%d",
        rIspCamInfo.rMapping_Info.eIspProfile,
        rIspCamInfo.rMapping_Info.eSensorMode,
        pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN,
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_EN,
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN
        );
*/
    CAM_LOGD_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);
    AAA_TRACE_END_HAL;

    return fgRet;
}

MBOOL
Paramctrl::
applyToHw_PerFrame_MFB(MINT32 /*flowType*/, const ISP_INFO_T& rIspInfo, void* pMfbBuf)
{

    MBOOL fgRet = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] + pMfbBuf(%p)", __FUNCTION__, pMfbBuf);

    const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;
    mfb_reg_t* pMfb = reinterpret_cast<mfb_reg_t*>(pMfbBuf);
    ::memset((void*)pMfb, 0, sizeof(mfb_reg_t));

    fgRet = MTRUE
        &&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rMapping_Info.eIspProfile, pMfb)
        ;

    // the protection of HW limitations
    //ispHWConstraintSet(pReg);   // the protection of HW limitations

    CAM_LOGD_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);


    return fgRet;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_All()
{
    MBOOL fgRet = MTRUE;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagStart); // Profiling Start.

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_RMM_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_RMG_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_LCE_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_UDM_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_NBC2_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).reset()
        ;

    if  ( ! fgRet )
    {
        CAM_LOGE("reset error");
        goto lbExit;
    }

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_SL2F()
        //DBS should be after SL2F
        &&  prepareHw_PerFrame_ADBS()
        &&  prepareHw_PerFrame_DBS()
        &&  prepareHw_PerFrame_OBC()
        &&  prepareHw_PerFrame_BPC()
        &&  prepareHw_PerFrame_NR1()
        &&  prepareHw_PerFrame_PDC()
        &&  prepareHw_PerFrame_RMM()
        &&  prepareHw_PerFrame_RMG()
        &&  prepareHw_PerFrame_RPG()
        //pass2
        &&  prepareHw_PerFrame_CCM()
        &&  prepareHw_PerFrame_LCS()
        // wait TSF
        &&  prepareHw_PerFrame_DCPN()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_CPN()

        ;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

    if  ( ! fgRet )
    {
        CAM_LOGE("prepareHw error");
        goto lbExit;
    }

lbExit:
    return  fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_All(const ISP_INFO_T& rIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] +", __FUNCTION__);
    MBOOL bUseIsoThreshEnable = ::property_get_int32("vendor.debug.isp.isoThresh.enable", 1);
    m_isoThresh = ::property_get_int32("vendor.debug.isp.isoThresh", 5);

    MINT32 i4RealISO = static_cast<MINT32>(rIspInfo.rCamInfo.rAEInfo.u4RealISOValue);
    MINT32 i4ISO_L = static_cast<MINT32>(rIspInfo.rCamInfo.eIdx_ISO_L);
    MINT32 i4ZoomRatio_x100 = static_cast<MINT32>(rIspInfo.rCamInfo.i4ZoomRatio_x100);
    MBOOL  b1Update = false;
    MBOOL  bRrzOn = rIspInfo.rCamInfo.rCropRzInfo.fgRRZOnOff;

    if(bUseIsoThreshEnable)
    {
        CAM_LOGD_IF(m_bDebugEnable, "[%s] m_bFirstPrepare(%d), i4ISO_L/m_i4PreISO_L(%d/%d), u4RealISO/m_i4PreRealISO(%d/%d), isoThresh(%d), m_i4PreRealISO*isoThresh/100(%d), i4ZoomRatio_x100/(%d)m_i4PreZoomRatio_x100(%d), eControlMode(%d)",
            __FUNCTION__, m_bFirstPrepare, i4ISO_L, m_i4PreISO_L, i4RealISO, m_i4PreRealISO, m_isoThresh, m_i4PreRealISO*m_isoThresh/100, i4ZoomRatio_x100, m_i4PreZoomRatio_x100, rIspInfo.rCamInfo.eControlMode);
        if(m_bFirstPrepare
            || i4ISO_L != m_i4PreISO_L
            || (abs(i4RealISO - m_i4PreRealISO) > m_i4PreRealISO*m_isoThresh/100) \
            || (rIspInfo.rCamInfo.rMapping_Info.eIspProfile != EIspProfile_Preview && rIspInfo.rCamInfo.rMapping_Info.eIspProfile != EIspProfile_Video)
            || i4ZoomRatio_x100 != m_i4PreZoomRatio_x100
            || rIspInfo.rCamInfo.rMapping_Info.eIspProfile != m_i4PrePrevProfile
            || bRrzOn != m_bPreRrzOn
            || m_bMappingQueryFlag
            || rIspInfo.rCamInfo.eControlMode == MTK_CONTROL_MODE_OFF)
        {
            CAM_LOGD_IF(m_bDebugEnable, "[%s] Use new HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
            b1Update = true;
            m_i4PreISO_L = i4ISO_L;
            m_i4PreRealISO = i4RealISO;
            m_bFirstPrepare = MFALSE;
            m_i4PreZoomRatio_x100 = i4ZoomRatio_x100;
            m_i4PrePrevProfile = static_cast<MINT32>(rIspInfo.rCamInfo.rMapping_Info.eIspProfile);
            m_bPreRrzOn = bRrzOn;
        }
        else
        {
            CAM_LOGD_IF(m_bDebugEnable, "[%s] Use previous HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
        }
    }
    else
    {
        b1Update = true;
    }
    m_bUseIsoThreshEnable = !b1Update;
    MBOOL fgRet = MTRUE
        &&  prepareHw_PerFrame_SL2G(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_ADBS_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_DBS_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_OBC_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_BPC_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_NR1_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_PDC_2(rIspInfo.rCamInfo) : MTRUE)
        &&  prepareHw_PerFrame_CPN_2(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_LSC_2(rIspInfo)
        &&  prepareHw_PerFrame_DCPN_2(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_PGN(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_SL2(rIspInfo)
        &&  (b1Update ? prepareHw_PerFrame_RNR(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_UDM(rIspInfo.rCamInfo) : MTRUE)
        &&  prepareHw_PerFrame_FLC(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_CCM(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_CCM_2(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_GGM(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_GGM_2(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_NDG(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_ANR(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_ANR_TBL(rIspInfo.rCamInfo) : MTRUE)
        &&  prepareHw_PerFrame_NDG2(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_ANR2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_CCR(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_ABF(rIspInfo.rCamInfo) : MTRUE)
        &&   prepareHw_PerFrame_BOK(rIspInfo.rCamInfo)
        &&   prepareHw_PerFrame_PCA(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_HFG(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_EE(rIspInfo.rCamInfo) : MTRUE)
        &&   prepareHw_PerFrame_EFFECT(rIspInfo.rCamInfo)
        &&   prepareHw_PerFrame_NR3D(rIspInfo.rCamInfo)
        &&   prepareHw_PerFrame_COLOR(rIspInfo.rCamInfo)
        //&&  (b1Update ? prepareHw_PerFrame_MFB(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_MIXER3(rIspInfo.rCamInfo) : MTRUE)
        //LCE should be after HLR_2
        &&   prepareHw_PerFrame_LCE(rIspInfo.rCamInfo)
        &&   (b1Update ? prepareHw_PerFrame_RMM_2(rIspInfo.rCamInfo) : MTRUE)
        &&   (b1Update ? prepareHw_PerFrame_RMG_2(rIspInfo.rCamInfo) : MTRUE)
        ;


    CAM_LOGD_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);

    return  fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// For dynamic bypass application
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_Partial()
{
    MBOOL fgRet = MTRUE;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagStart); // Profiling Start.

    //  (1) reset: read register setting to ispmgr
    fgRet = MTRUE
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).reset()
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).reset()
            ;

    //  Exception of dynamic CCM
    if(isDynamicCCM())
        fgRet &= ISP_MGR_CCM_T::getInstance(m_eSensorDev).reset();


    if  ( ! fgRet )
    {
        goto lbExit;
    }

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_DynamicBypass_OBC(0)  //OBC pass1
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_RPG()
            ;


    //Exception of dynamic CCM
    if(isDynamicCCM())
        fgRet &= prepareHw_PerFrame_CCM();


    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

    if  ( ! fgRet )
    {
        goto lbExit;
    }

lbExit:
    CAM_LOGD_IF(m_bDebugEnable, "[prepareHw_PerFrame_Partial()] exit\n");
    return  fgRet;

}

MBOOL
Paramctrl::
prepareHw_PerFrame_Partial(const ISP_INFO_T& rIspInfo)
{
    prepareHw_PerFrame_UDM(rIspInfo.rCamInfo);
    prepareHw_PerFrame_RNR(rIspInfo.rCamInfo);
    prepareHw_PerFrame_PGN(rIspInfo.rCamInfo);
    prepareHw_DynamicBypass_OBC(1);  //obc pass2
    prepareHw_PerFrame_LSC_2(rIspInfo);

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Paramctrl::
ispHWConstraintSet(dip_x_reg_t* pReg)
{

    MBOOL bBNR2_EN = pReg->DIP_X_CTL_RGB_EN.Bits.BNR2_EN;
    MBOOL bBPC2_EN = pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_EN;

    //Limit 1
    if (bBNR2_EN && (bBPC2_EN == 0))
    {
        pReg->DIP_X_CTL_RGB_EN.Bits.BNR2_EN = 0;
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_EN = 0;
        pReg->DIP_X_BNR2_NR1_CON.Bits.NR1_CT_EN = 0;
        //pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN = 0;

        CAM_LOGD_IF(m_bDebugEnable, "BNR2::apply warning: set (bBNR2_EN, bBPC2_EN) = (%d, %d)", bBNR2_EN, bBPC2_EN);
    }

    MBOOL bDBS2_EN = pReg->DIP_X_CTL_RGB_EN.Bits.DBS2_EN;

    if(bDBS2_EN){
        pReg->DIP_X_CTL_RGB2_EN.Bits.ADBS2_EN = 0;
    }

    MBOOL bUDM_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.UDM_EN;
    MBOOL bNBC_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.NBC_EN;
    MBOOL bNBC2_EN = pReg->DIP_X_CTL_YUV_EN.Bits.NBC2_EN;
    MBOOL bSEEE_EN = pReg->DIP_X_CTL_YUV_EN.Bits.SEEE_EN;
    MBOOL bRNR_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.RNR_EN;
    MBOOL bHFG_EN  = pReg->DIP_X_CTL_YUV2_EN.Bits.HFG_EN;
    MBOOL bNR3D_EN = pReg->DIP_X_CAM_TNR_ENG_CON.Bits.NR3D_CAM_TNR_EN;
    MBOOL bSL2G_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.SL2G_EN;

    MBOOL bSL2_EN   = pReg->DIP_X_CTL_RGB_EN.Bits.SL2_EN;
    MBOOL bSL2B_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2B_EN;
    MBOOL bSL2C_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2C_EN;
    MBOOL bSL2D_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2D_EN;
    MBOOL bSL2E_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2E_EN;
    MBOOL bSL2H_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.SL2H_EN;
    MBOOL bSL2I_EN  = pReg->DIP_X_CTL_YUV2_EN.Bits.SL2I_EN;

    //Limit 2
    if (!bUDM_EN){
        pReg->DIP_X_CTL_RGB_EN.Bits.SL2_EN = 0;
    }
    else if (!bSL2_EN){
        pReg->DIP_X_UDM_SL_CTL.Bits.UDM_SL_EN = 0;
    }

    //Limit 3
    if (!bNBC_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2B_EN = 0;
    }
    else if(!bSL2B_EN){
        pReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_SL2_LINK = 0;
    }

    //Limit 4
    if (!bNBC2_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2C_EN = 0;
    }
    else if(!bSL2C_EN){
        pReg->DIP_X_NBC2_ANR2_CON1.Bits.NBC2_ANR2_SL2_LINK = 0;
        pReg->DIP_X_NBC2_CCR_CON.Bits.NBC2_CCR_SL2_LINK = 0;
    }

    //Limit 5
    if (!bSEEE_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2D_EN = 0;
    }
    else if(!bSL2D_EN){
        pReg->DIP_X_SEEE_LUMA_SLNK_CTRL.Bits.SEEE_GLUT_LINK_EN = 0;
    }

    //Limit 6
    if (!bNR3D_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2E_EN = 0;
    }
    else if(!bSL2E_EN){
        pReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF = 1;
    }

    //Limit 7
    if (!bRNR_EN){
        pReg->DIP_X_CTL_RGB_EN.Bits.SL2H_EN = 0;
    }
    else if(!bSL2H_EN){
        pReg->DIP_X_RNR_SL.Bits.RNR_SL_EN = 0;
    }

    //Limit 8
    if (!bHFG_EN){
        pReg->DIP_X_CTL_YUV2_EN.Bits.SL2I_EN = 0;
    }
    else if(!bSL2I_EN){
        pReg->DIP_X_CTL_YUV2_EN.Bits.HFG_EN = 0;
    }

    //Limit 10
    if (!bDBS2_EN){
        pReg->DIP_X_CTL_RGB_EN.Bits.SL2G_EN = 0;
    }else if(bSL2G_EN){
        pReg->DIP_X_DBS2_CTL.Bits.DBS_SL_EN = 0;
    }


//HHHH  BOK mode N3D SRZ3
    //Limit 10
    if (bNBC2_EN ){
        //pReg->DIP_X_ANR2_CON1.Bits.ANR2_MODE = 0;
    }

#if 0
    //PD sensor
    MBOOL bPDC_EN = pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN;
    // PDAF SW limitation
    if (bPDC_EN){
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 1;
    }
#endif

    if(ISP_MGR_BNR2_T::getInstance(m_eSensorDev).getDMGItable() == NULL &&
       ISP_MGR_BNR2_T::getInstance(m_eSensorDev).getDMGItable_Default() == NULL){
       pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 0;
    }

    if(pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN == 0){
        pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN = 0;
    }

    MBOOL bCOLOR_EN = pReg->DIP_X_CTL_YUV_EN.Bits.COLOR_EN;
    pReg->DIP_X_CAM_COLOR_START.Bits.COLOR_DISP_COLOR_START = bCOLOR_EN;

    MBOOL bLSC2_EN = pReg->DIP_X_CTL_RGB_EN.Bits.LSC2_EN;
    pReg->DIP_X_CTL_RGB2_EN.Bits.CPN2_EN = bLSC2_EN;
    pReg->DIP_X_CTL_RGB2_EN.Bits.DCPN2_EN = bLSC2_EN;

    MBOOL bLCE_EN = pReg->DIP_X_CTL_RGB_EN.Bits.LCE_EN;
    pReg->DIP_X_CTL_MISC_SEL.Bits.NBC_GMAP_LTM_MODE = bLCE_EN;

    MBOOL bCondition = bNBC_EN && ( pReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_ENC ||
                                    pReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_ENY);

    if(!(bCondition && pReg->DIP_X_CTL_MISC_SEL.Bits.NBC_GMAP_LTM_MODE)){
        pReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_LTM_LINK = 0;
    }

    if(bCondition &&  bNBC2_EN && pReg->DIP_X_NBC2_ANR2_CON1.Bits.NBC2_ANR2_MODE){
        pReg->DIP_X_NBC_ANR_CON1.Bits.NBC_ANR_LTM_LINK = 0;
    }

    pReg->DIP_X_ADBS2_CTL.Bits.ADBS_LE_INV_CTL = 0;
    pReg->DIP_X_DBS2_CTL.Bits.DBS_LE_INV_CTL = 0;
    pReg->DIP_X_BNR2_BPC_CON.Bits.BNR_LE_INV_CTL = 0;
    pReg->DIP_X_RMG2_HDR_GAIN.Bits.RMG_LE_INV_CTL = 0;
    pReg->DIP_X_RMM2_OSC.Bits.RMM_LE_INV_CTL = 0;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Paramctrl::
prepareHw_PerFrame_Default()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
updateLscInfo(MINT32 i4IsLockRto)
{
    MUINT32 new_cct_idx = eIDX_Shading_CCT_BEGIN;

    MINT32 i4Rto = ::property_get_int32("vendor.debug.lsc_mgr.ratio", -1);
    MINT32 i4forceSuperNightMode = ::property_get_int32("vendor.lsc.force.superNight", 0);

    MBOOL fgSuperNightModeEn = (m_bIsSuperNightMode||i4forceSuperNightMode);
    MBOOL fgForceUn = (i4IsLockRto == 2) && m_rIspCamInfo.rMapping_Info.eFlash;
    MBOOL fgLockRto = (m_rIspCamInfo.rAEInfo.bAELock || m_rIspCamInfo.rAEInfo.bAELimiter || i4IsLockRto || m_rIspCamInfo.rManualAE) && !fgForceUn && !fgSuperNightModeEn;
    MBOOL fgAutoRto = (i4Rto == -1);

    // Invoke callback for customers to modify.
    if  (m_fgDynamicShading)
    {
        // Dynamic Tuning: Enable
        new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo);
        m_pLscMgr->setCTIdx(new_cct_idx);

        if (fgAutoRto && !fgLockRto)
        {
            AAA_TRACE_LSC(evaluate_Shading_Ratio);
            i4Rto = m_pIspTuningCustom->evaluate_Shading_Ratio(m_rIspCamInfo, fgSuperNightModeEn);
            AAA_TRACE_END_LSC;
            m_pLscMgr->setRatio(i4Rto);

            if(fgSuperNightModeEn)
            {
                CAM_LOGD_IF(m_bDebugEnable, "[%s] for shading valid to apply ratio", __FUNCTION__);
                ISP_MGR_LSC_T::getInstance(m_eSensorDev).putRatio(i4Rto);
            }
        }
        else if (!fgAutoRto)
        {
            m_pLscMgr->setRatio(i4Rto);
        }
        // else: lock, do not update ratio
    }
    else
    {
        if (fgAutoRto) i4Rto = 32;
        m_pLscMgr->setRatio(i4Rto);
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] (flash, iso, rto)=(%d, %d, %d), auto(%d), lock(%d,%d,%d), dyn(%d), op(%d), PreCapFlashIsLockRto(%d), super night En(%d), ev (%d), forceUnlock(%d)", __FUNCTION__,
        m_rIspCamInfo.rFlashInfo.isFlash, m_rIspCamInfo.rAEInfo.u4RealISOValue, i4Rto, fgAutoRto,
        m_rIspCamInfo.rAEInfo.bAELock, m_rIspCamInfo.rAEInfo.bAELimiter, m_rIspCamInfo.rManualAE, m_fgDynamicShading, getOperMode(), i4IsLockRto,
        fgSuperNightModeEn, m_rIspCamInfo.rAEInfo.i4AEComp, fgForceUn);

    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    //////////////////////////////////////
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_DCPN()
{
    ISP_MGR_DCPN_T& rDcpn = ISP_MGR_DCPN_T::getInstance(m_eSensorDev);

    rDcpn.setEnable(MTRUE);

    if(rDcpn.isEnable() && (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_RMG_EN)) {
        rDcpn.setZHdrEnable(MTRUE);
    }
    else {
        rDcpn.setZHdrEnable(MFALSE); //bypass mode
    }

    //Force set ZHdrEnable
    MINT32 forceZhdrEn = ::property_get_int32("vendor.debug.forceset.isp.zhdr.enable", -1);
    if(forceZhdrEn != -1)
    {
        if(forceZhdrEn == 0)
            rDcpn.setZHdrEnable(MFALSE); //bypass mode
        if(forceZhdrEn == 1)
            rDcpn.setZHdrEnable(MTRUE);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC()
{
    MY_LOG_IF(m_bDebugEnable, "%s", __FUNCTION__);
    MINT32 defaultLSCValue = -1;
#if CAM3_DISABLE_SHADING
    defaultLSCValue = 0;
#endif
    //////////////////////////////////////
    MINT32 i4Tsf = ::property_get_int32("vendor.debug.lsc_mgr.manual_tsf", defaultLSCValue);
    MINT32 i4OnOff = ::property_get_int32("vendor.debug.lsc_mgr.enable", defaultLSCValue);

    if (i4Tsf != -1){
        m_pLscMgr->setTsfOnOff(i4Tsf ? MTRUE : MFALSE);
    }

    if (i4OnOff != -1){
        m_pLscMgr->setOnOff(i4OnOff ? MTRUE : MFALSE);
    }
    CAM_LOGD_IF(m_bDebugEnable,"%s(): bAELock:%d eToneMapMode:%d\n", __FUNCTION__, m_rIspCamInfo.rAEInfo.bAELock, m_rIspCamInfo.eToneMapMode);
    if (m_rIspCamInfo.rAEInfo.bAELock && m_rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
    {
        m_pLscMgr->setOnOff(MFALSE);
    }

    if (!m_pLscMgr->getTsfOnOff())
        m_pLscMgr->updateLsc();
    else
        m_pLscMgr->waitTsfExecDone();

    ILscMgr::SL2_CFG_T rSl2Cfg = m_pLscMgr->getSl2();
    MUINT32 u4RawWd, u4RawHt;
    m_pLscMgr->getRawSize(m_pLscMgr->getSensorMode(), u4RawWd, u4RawHt);
    m_rIspCamInfo.rCropRzInfo.i4TGoutW = u4RawWd;
    m_rIspCamInfo.rCropRzInfo.i4TGoutH = u4RawHt;
    m_rIspCamInfo.rSl2Info.i4CenterX  = rSl2Cfg.i4CenterX;
    m_rIspCamInfo.rSl2Info.i4CenterY  = rSl2Cfg.i4CenterY;
    m_rIspCamInfo.rSl2Info.i4R0       = rSl2Cfg.i4R0;
    m_rIspCamInfo.rSl2Info.i4R1       = rSl2Cfg.i4R1;
    m_rIspCamInfo.rSl2Info.i4R2       = rSl2Cfg.i4R2;
    //////////////////////////////////////

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_CPN()
{
    ISP_MGR_CPN_T& rCpn = ISP_MGR_CPN_T::getInstance(m_eSensorDev);

    rCpn.setEnable(MTRUE);

    if(rCpn.isEnable() && (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_RMG_EN)) {
        rCpn.setZHdrEnable(MTRUE);
    }
    else {
        rCpn.setZHdrEnable(MFALSE); //bypass mode
    }

    //Force set ZHdrEnable
    MINT32 forceZhdrEn = ::property_get_int32("vendor.debug.forceset.isp.zhdr.enable", -1);
    if(forceZhdrEn != -1)
    {
        if(forceZhdrEn == 0)
            rCpn.setZHdrEnable(MFALSE); //bypass mode
        if(forceZhdrEn == 1)
            rCpn.setZHdrEnable(MTRUE);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_SL2F()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2f.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_SL2F_T& rSl2f = ISP_MGR_SL2F_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgSL2F = rSl2f.isCCTEnable();

    if ((bDisable) || (!rSl2f.isCCTEnable()) ||
        (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2FG_EN)))
    {
        rSl2f.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rSl2f.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_SL2_T sl2f = m_rIspParam.ISPRegs.SL2F[m_ParamIdx_P1.SL2F];

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_SL2F(m_rIspCamInfo, m_IspNvramMgr, sl2f);
        }

        // Load it to ISP manager buffer.
        rSl2f.put(sl2f);
    }

    return  MTRUE;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ADBS()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.adbs.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_ADBS_T& rAdbs = ISP_MGR_ADBS_T::getInstance(m_eSensorDev);
    MBOOL bIsZHDRTuning = m_pIspTuningCustom->is_ZHDRTuning(m_rIspCamInfo);

    //m_rIspCamInfo.fgCCTInfo.Bits.fgDBS = rDbs.isCCTEnable();

    if ((bDisable)||(!rAdbs.isCCTEnable()) || bIsZHDRTuning || (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_ADBS_EN))|| m_DualPD_PureRaw )
    {
        rAdbs.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rAdbs.setEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs.u2LowerIso_LowerZoom, m_bMappingQueryFlagP1);
        ISP_NVRAM_ADBS_T adbs = m_rIspParam.ISPRegs.ADBS[m_ISP_INT.sAdbs.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(m_rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , m_rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs.u2UpperIso_UpperZoom, m_bMappingQueryFlagP1);
            //getISPIndex(eIDX_LowerIso_UpperZoom , m_rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs.u2LowerIso_UpperZoom, m_bMappingQueryFlagP1);
            getISPIndex(eIDX_UpperIso_LowerZoom , m_rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs.u2UpperIso_LowerZoom, m_bMappingQueryFlagP1);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sAdbs.u2UpperIso_UpperZoom,
                     m_ISP_INT.sAdbs.u2LowerIso_UpperZoom,
                     m_ISP_INT.sAdbs.u2UpperIso_LowerZoom,
                     m_ISP_INT.sAdbs.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothADBS);

            SmoothADBS(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.ADBS[m_ISP_INT.sAdbs.u2UpperIso_LowerZoom], // ADBS settings for upper ISO
                    m_rIspParam.ISPRegs.ADBS[m_ISP_INT.sAdbs.u2LowerIso_LowerZoom], // ADBS settings for lower ISO
                    adbs);  // Output

            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_ADBS, m_ParamIdx_P1.ADBS, m_bMappingQueryFlagP1);
            adbs = m_rIspParam.ISPRegs.ADBS[m_ParamIdx_P1.ADBS];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            //m_pIspTuningCustom->refine_DBS(m_rIspCamInfo, m_IspNvramMgr, adbs);
        }

        // Load it to ISP manager buffer.
        rAdbs.put(adbs);
    }

    return  MTRUE;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_DBS()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.dbs.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_DBS_T& rDbs = ISP_MGR_DBS_T::getInstance(m_eSensorDev);
    MBOOL bIsZHDRTuning = m_pIspTuningCustom->is_ZHDRTuning(m_rIspCamInfo);
    m_rIspCamInfo.fgCCTInfo.Bits.fgDBS = rDbs.isCCTEnable();

    if ((bDisable) || !(bIsZHDRTuning) || (!rDbs.isCCTEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_DBS_EN))|| m_DualPD_PureRaw )
    {
        rDbs.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rDbs.setEnable(MTRUE);

        // Get default NVRAM parameter
        RAWIspCamInfo temp_CamInfo = m_rIspCamInfo;
        getISPIndex(eIDX_LowerIso_LowerZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs.u2LowerIso_LowerZoom, m_bMappingQueryFlagP1);
        ISP_NVRAM_DBS_T dbs = m_rIspParam.ISPRegs.DBS[m_ISP_INT.sDbs.u2LowerIso_LowerZoom];

        // HDR Flow Setting
        temp_CamInfo.rAEInfo.u4RealISOValue = m_pIspTuningCustom->get_HdrDbsNewISO(m_rIspCamInfo);
        temp_CamInfo.rMapping_Info.eISO_Idx = map_ISO_value2index(temp_CamInfo.rAEInfo.u4RealISOValue);
        get_ISOIdx_neighbor(temp_CamInfo, temp_CamInfo.rMapping_Info.eISO_Idx, temp_CamInfo.rAEInfo.u4RealISOValue);

        //Interpolation ISP module
        if (isIspInterpolation(temp_CamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs.u2UpperIso_UpperZoom, m_bMappingQueryFlagP1);
            //getISPIndex(eIDX_LowerIso_UpperZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs.u2LowerIso_UpperZoom, m_bMappingQueryFlagP1);
            getISPIndex(eIDX_UpperIso_LowerZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs.u2UpperIso_LowerZoom, m_bMappingQueryFlagP1);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): customized DBS_Iso = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, temp_CamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sDbs.u2UpperIso_UpperZoom,
                     m_ISP_INT.sDbs.u2LowerIso_UpperZoom,
                     m_ISP_INT.sDbs.u2UpperIso_LowerZoom,
                     m_ISP_INT.sDbs.u2LowerIso_LowerZoom);

            SmoothDBS(temp_CamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.DBS[m_ISP_INT.sDbs.u2UpperIso_LowerZoom], // DBS settings for upper ISO
                    m_rIspParam.ISPRegs.DBS[m_ISP_INT.sDbs.u2LowerIso_LowerZoom], // DBS settings for lower ISO
                    dbs);  // Output

            AAA_TRACE_END_ALG;
        }
        else{
             getISPIndex(eIDX_LowerIso_LowerZoom, temp_CamInfo, EModule_DBS, m_ParamIdx_P1.DBS, m_bMappingQueryFlagP1);
             dbs = m_rIspParam.ISPRegs.DBS[m_ParamIdx_P1.DBS];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            //m_pIspTuningCustom->refine_DBS(m_rIspCamInfo, m_IspNvramMgr, dbs);
        }

        // Load it to ISP manager buffer.
        rDbs.put(dbs);
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_DBS(m_rIspCamInfo, m_IspNvramMgr, dbs);
        }

        // Load it to ISP manager buffer.
        rDbs.put(dbs);
    }

    return  MTRUE;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_OBC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.obc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_OBC_T& rObc = ISP_MGR_OBC_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgOBC = rObc.isCCTEnable();

    if ((bDisable) || (!rObc.isCCTEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_OBC_EN)) || m_DualPD_PureRaw)
    {
        rObc.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rObc.setEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_OBC, m_ISP_INT.sObc.u2LowerIso_LowerZoom, m_bMappingQueryFlagP1);
        ISP_NVRAM_OBC_T obc = m_rIspParam.ISPRegs.OBC[m_ISP_INT.sObc.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(m_rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , m_rIspCamInfo, EModule_OBC, m_ISP_INT.sObc.u2UpperIso_UpperZoom, m_bMappingQueryFlagP1);
            //getISPIndex(eIDX_LowerIso_UpperZoom , m_rIspCamInfo, EModule_OBC, m_ISP_INT.sObc.u2LowerIso_UpperZoom, m_bMappingQueryFlagP1);
            getISPIndex(eIDX_UpperIso_LowerZoom , m_rIspCamInfo, EModule_OBC, m_ISP_INT.sObc.u2UpperIso_LowerZoom, m_bMappingQueryFlagP1);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sObc.u2UpperIso_UpperZoom,
                     m_ISP_INT.sObc.u2LowerIso_UpperZoom,
                     m_ISP_INT.sObc.u2UpperIso_LowerZoom,
                     m_ISP_INT.sObc.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothOBC);
            SmoothOBC(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.OBC[m_ISP_INT.sObc.u2UpperIso_LowerZoom], // OBC settings for upper ISO
                    m_rIspParam.ISPRegs.OBC[m_ISP_INT.sObc.u2LowerIso_LowerZoom], // OBC settings for lower ISO
                    obc);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_OBC, m_ParamIdx_P1.OBC, m_bMappingQueryFlagP1);
            obc = m_rIspParam.ISPRegs.OBC[m_ParamIdx_P1.OBC];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_OBC(m_rIspCamInfo, m_IspNvramMgr, obc);
        }

        this->setPureOBCInfo(&obc);

        // Load it to ISP manager buffer.
        rObc.put(obc);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_DynamicBypass_OBC(MBOOL fgOBPass)
{
    // Get backup NVRAM parameter
    ISP_NVRAM_OBC_T obc;

    this->getPureOBCInfo(&obc);

    // Load it to ISP manager buffer.
    if(!fgOBPass){
        ISP_MGR_OBC_T::getInstance(m_eSensorDev).put(obc);
    }
    else{
        ISP_MGR_OBC2_T::getInstance(m_eSensorDev).put(obc);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_BPC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.bpc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_BNR_T& rBnr = ISP_MGR_BNR_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgBPC = rBnr.isCCTBPCEnable();

    if ((bDisable) || (!rBnr.isCCTBPCEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_BPC_EN))|| m_DualPD_PureRaw)
    {
        rBnr.setBPCEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rBnr.setBPCEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc.u2LowerIso_LowerZoom, m_bMappingQueryFlagP1);
        ISP_NVRAM_BNR_BPC_T bpc = m_rIspParam.ISPRegs.BNR_BPC[m_ISP_INT.sBpc.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(m_rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , m_rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc.u2UpperIso_UpperZoom, m_bMappingQueryFlagP1);
            //getISPIndex(eIDX_LowerIso_UpperZoom , m_rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc.u2LowerIso_UpperZoom, m_bMappingQueryFlagP1);
            getISPIndex(eIDX_UpperIso_LowerZoom , m_rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc.u2UpperIso_LowerZoom, m_bMappingQueryFlagP1);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sBpc.u2UpperIso_UpperZoom,
                     m_ISP_INT.sBpc.u2LowerIso_UpperZoom,
                     m_ISP_INT.sBpc.u2UpperIso_LowerZoom,
                     m_ISP_INT.sBpc.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothBPC);
            SmoothBPC(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.BNR_BPC[m_ISP_INT.sBpc.u2UpperIso_LowerZoom], // BPC settings for upper ISO
                    m_rIspParam.ISPRegs.BNR_BPC[m_ISP_INT.sBpc.u2LowerIso_LowerZoom], // BPC settings for lower ISO
                    bpc);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_BNR_BPC, m_ParamIdx_P1.BNR_BPC, m_bMappingQueryFlagP1);
            bpc = m_rIspParam.ISPRegs.BNR_BPC[m_ParamIdx_P1.BNR_BPC];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BPC(m_rIspCamInfo, m_IspNvramMgr, bpc);
        }

        // Load it to ISP manager buffer.
        rBnr.put(bpc);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR1()
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.nr1.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_BNR_T& rBnr = ISP_MGR_BNR_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgNR1 = rBnr.isCCTCTEnable();

    if ((bDisable) || (!rBnr.isCCTCTEnable()) ||
        (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_NR1_EN)) ||
        (m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
        (m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG) ||
         m_DualPD_PureRaw)
    {
        rBnr.setCTEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            rBnr.setCTEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr1.u2LowerIso_LowerZoom, m_bMappingQueryFlagP1);
        ISP_NVRAM_BNR_NR1_T nr1 = m_rIspParam.ISPRegs.BNR_NR1[m_ISP_INT.sNr1.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(m_rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , m_rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr1.u2UpperIso_UpperZoom, m_bMappingQueryFlagP1);
            //getISPIndex(eIDX_LowerIso_UpperZoom , m_rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr1.u2LowerIso_UpperZoom, m_bMappingQueryFlagP1);
            getISPIndex(eIDX_UpperIso_LowerZoom , m_rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr1.u2UpperIso_LowerZoom, m_bMappingQueryFlagP1);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sNr1.u2UpperIso_UpperZoom,
                     m_ISP_INT.sNr1.u2LowerIso_UpperZoom,
                     m_ISP_INT.sNr1.u2UpperIso_LowerZoom,
                     m_ISP_INT.sNr1.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothNR1);
            SmoothNR1(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.BNR_NR1[m_ISP_INT.sNr1.u2UpperIso_LowerZoom], // NR1 settings for upper ISO
                    m_rIspParam.ISPRegs.BNR_NR1[m_ISP_INT.sNr1.u2LowerIso_LowerZoom], // NR1 settings for lower ISO
                    nr1);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_BNR_NR1, m_ParamIdx_P1.BNR_NR1, m_bMappingQueryFlagP1);
            nr1 = m_rIspParam.ISPRegs.BNR_NR1[m_ParamIdx_P1.BNR_NR1];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_NR1(m_rIspCamInfo, m_IspNvramMgr, nr1);
        }

        // Load it to ISP manager buffer.
        rBnr.put(nr1);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PDC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.pdc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_MGR_BNR_T& rBnr = ISP_MGR_BNR_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgPDC = rBnr.isCCTPDCEnable();

    if( (bDisable)||
        (!rBnr.isCCTPDCEnable())  ||
         m_DualPD_PureRaw )
    {
        rBnr.setPDCEnable(MFALSE);
    }
    else{

        //if (getOperMode() != EOperMode_Meta)
        rBnr.setPDCEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc.u2LowerIso_LowerZoom, m_bMappingQueryFlagP1);
        ISP_NVRAM_BNR_PDC_T pdc = m_rIspParam.ISPRegs.BNR_PDC[m_ISP_INT.sPdc.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(m_rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , m_rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc.u2UpperIso_UpperZoom, m_bMappingQueryFlagP1);
            //getISPIndex(eIDX_LowerIso_UpperZoom , m_rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc.u2LowerIso_UpperZoom, m_bMappingQueryFlagP1);
            getISPIndex(eIDX_UpperIso_LowerZoom , m_rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc.u2UpperIso_LowerZoom, m_bMappingQueryFlagP1);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sPdc.u2UpperIso_UpperZoom,
                     m_ISP_INT.sPdc.u2LowerIso_UpperZoom,
                     m_ISP_INT.sPdc.u2UpperIso_LowerZoom,
                     m_ISP_INT.sPdc.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothPDC);
            SmoothPDC(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.BNR_PDC[m_ISP_INT.sPdc.u2UpperIso_LowerZoom], // PDC settings for upper ISO
                    m_rIspParam.ISPRegs.BNR_PDC[m_ISP_INT.sPdc.u2LowerIso_LowerZoom], // PDC settings for lower ISO
                    pdc);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_BNR_PDC, m_ParamIdx_P1.BNR_PDC, m_bMappingQueryFlagP1);
            pdc = m_rIspParam.ISPRegs.BNR_PDC[m_ParamIdx_P1.BNR_PDC];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_PDC(m_rIspCamInfo, m_IspNvramMgr, pdc);
        }

        // Load it to ISP manager buffer.
        rBnr.put(pdc);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RMM()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_RMM_T& rRmm = ISP_MGR_RMM_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgRMM = rRmm.isCCTEnable();

    if ((bDisable) || (!rRmm.isCCTEnable()) ||
        (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_RMM_EN)))
    {
        rRmm.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rRmm.setEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm.u2LowerIso_LowerZoom, m_bMappingQueryFlagP1);
        ISP_NVRAM_RMM_T rmm = m_rIspParam.ISPRegs.RMM[m_ISP_INT.sRmm.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(m_rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , m_rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm.u2UpperIso_UpperZoom, m_bMappingQueryFlagP1);
            //getISPIndex(eIDX_LowerIso_UpperZoom , m_rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm.u2LowerIso_UpperZoom, m_bMappingQueryFlagP1);
            getISPIndex(eIDX_UpperIso_LowerZoom , m_rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm.u2UpperIso_LowerZoom, m_bMappingQueryFlagP1);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                        __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                         m_ISP_INT.sRmm.u2UpperIso_UpperZoom,
                         m_ISP_INT.sRmm.u2LowerIso_UpperZoom,
                         m_ISP_INT.sRmm.u2UpperIso_LowerZoom,
                         m_ISP_INT.sRmm.u2LowerIso_LowerZoom);


            AAA_TRACE_ALG(SmoothRMM);
            SmoothRMM(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                       m_rIspParam.ISPRegs.RMM[m_ISP_INT.sRmm.u2UpperIso_LowerZoom], // RMM settings for upper ISO
                       m_rIspParam.ISPRegs.RMM[m_ISP_INT.sRmm.u2LowerIso_LowerZoom], // RMM settings for lower ISO
                       rmm);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , m_rIspCamInfo, EModule_RMM, m_ParamIdx_P1.RMM, m_bMappingQueryFlagP1);
            rmm = m_rIspParam.ISPRegs.RMM[m_ParamIdx_P1.RMM];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_RMM(m_rIspCamInfo, m_IspNvramMgr, rmm);
        }

        // Load it to ISP manager buffer.
        ISP_MGR_RMM_T::getInstance(m_eSensorDev).put(rmm);
    }
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RMG()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_RMG_T& rRmg = ISP_MGR_RMG_T::getInstance(m_eSensorDev);

    if ((bDisable) || (!(m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_RMG_EN)))
    {
        rRmg.setEnable(MFALSE);
    }
    else{
        rRmg.setEnable(MTRUE);
    }
#endif

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RPG()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rpg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_RPG_T& rRpg = ISP_MGR_RPG_T::getInstance(m_eSensorDev);
    // Get default NVRAM parameter
    ISP_NVRAM_RPG_T rpg = {0};

    AWB_GAIN_T rCurrentAWBGain = m_rIspCamInfo.rAWBInfo.rRPG;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] En(%d), AWB(%d,%d,%d)", __FUNCTION__,
        isRPGEnable(), rCurrentAWBGain.i4R, rCurrentAWBGain.i4G, rCurrentAWBGain.i4B);

    rRpg.setIspAWBGain(rCurrentAWBGain);

    if ((isRPGEnable() && (!bDisable)) && (!m_DualPD_PureRaw) &&
        (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_RPGN_EN))
    {
        rRpg.setEnable(MTRUE);
        getIspHWBuf(m_eSensorDev, rpg );

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_RPG(m_rIspCamInfo, m_IspNvramMgr, rpg);
        }

        // Load it to ISP manager buffer.
        rRpg.put(rpg);
    }
    else {
        rRpg.setEnable(MFALSE);
    }

    return  MTRUE;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_NVRAM_CCM_T rCCM;
    ::memset(&rCCM, 0, sizeof(ISP_NVRAM_CCM_T));

    if(m_rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX){
        rCCM = m_pCcmMgr->getCCM();
    }
    else if (isDynamicCCM() &&(!m_rIspCamInfo.rAWBInfo.bAWBLock))
    {
        MY_LOG_IF(m_bDebugEnable, "Smooth CCM");

        getISPIndex(eIDX_LowerLV, m_rIspCamInfo, EModule_CCM, m_ISP_INT.sCCM.u2LowerLv_LowerCt, m_bMappingQueryFlagP1);
        getISPIndex(eIDX_UpperLV, m_rIspCamInfo, EModule_CCM, m_ISP_INT.sCCM.u2UpperLv_LowerCt, m_bMappingQueryFlagP1);

        CAM_LOGD_IF(m_bDebugEnable,"%s_1(): i4RealLightValue_x10 = %d, u2LowerLv = %d, u2UpperLv = %d\n",
                   __FUNCTION__, m_rIspCamInfo.rAEInfo.i4RealLightValue_x10,
                   m_ISP_INT.sCCM.u2LowerLv_LowerCt,
                   m_ISP_INT.sCCM.u2UpperLv_LowerCt);
        AAA_TRACE_ALG(calculateCCM);

        m_pCcmMgr->calculateCCM(rCCM, m_ISP_ALG_VER.sCCM,
            m_rIspCamInfo,
            m_rIspParam.ISPColorTbl.CCM[m_ISP_INT.sCCM.u2LowerLv_LowerCt],
            m_rIspParam.ISPColorTbl.CCM[m_ISP_INT.sCCM.u2UpperLv_LowerCt],
            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[m_rIspCamInfo.eIdx_LV_U],
            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[m_rIspCamInfo.eIdx_LV_L]);
        AAA_TRACE_END_ALG;

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, rCCM);
        }
    }
    else {
        rCCM = m_pCcmMgr->getPrvCCM();//take output directly
    }

    m_pCcmMgr->updateCCM(rCCM);//for auto ccm result

    m_rIspCamInfo.rMtkCCM = rCCM;
    return  MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_LCS(){

    return  MTRUE;

}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// prepare HW in pass 2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_SL2G(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2g.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgSl2gEn = fgPureRaw && fgPgnEn;
    ISP_MGR_SL2G_T& rSl2g = ISP_MGR_SL2G_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgSl2gEn|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgSL2F)||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2FG_EN)))
    {
        rSl2g.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rSl2g.setEnable(MTRUE);

        getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo, EModule_SL2F, m_ISP_INT.u2SL2F, m_bMappingQueryFlag);
        ISP_NVRAM_SL2_T sl2g = m_rIspParam.ISPRegs.SL2F[m_ISP_INT.u2SL2F];

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_SL2F(rIspCamInfo, m_IspNvramMgr, sl2g);
        }

        // Load it to ISP manager buffer.
        rSl2g.put(sl2g);

    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// prepare HW in pass 2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ADBS_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.adbs2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL bIsZHDRTuning = m_pIspTuningCustom->is_ZHDRTuning(rIspCamInfo);

    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgADbs2En = fgPureRaw && fgPgnEn;
    ISP_MGR_ADBS2_T& rAdbs2 = ISP_MGR_ADBS2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgADbs2En|| bDisable || bIsZHDRTuning ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_ADBS_EN)))
    {
        rAdbs2.setEnable(MFALSE);
    }
    else
    {
        rAdbs2.setEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs2.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_ADBS_T adbs = m_rIspParam.ISPRegs.ADBS[m_ISP_INT.sAdbs2.u2LowerIso_LowerZoom];

        ISP_NVRAM_PGN_T pgn;
        getIspHWBuf(m_eSensorDev, pgn );

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs2.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs2.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_ADBS, m_ISP_INT.sAdbs2.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sAdbs2.u2UpperIso_UpperZoom,
                     m_ISP_INT.sAdbs2.u2LowerIso_UpperZoom,
                     m_ISP_INT.sAdbs2.u2UpperIso_LowerZoom,
                     m_ISP_INT.sAdbs2.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothADBS_2);

            SmoothADBS(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.ADBS[m_ISP_INT.sAdbs2.u2UpperIso_LowerZoom], // DBS settings for upper ISO
                    m_rIspParam.ISPRegs.ADBS[m_ISP_INT.sAdbs2.u2LowerIso_LowerZoom], // DBS settings for lower ISO
                    adbs);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_ADBS, m_ParamIdx_P2.ADBS, m_bMappingQueryFlag);
            adbs = m_rIspParam.ISPRegs.ADBS[m_ParamIdx_P2.ADBS];
        }
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            //m_pIspTuningCustom->refine_DBS(rIspCamInfo, m_IspNvramMgr, adbs);
        }
        // Load it to ISP manager buffer.
        rAdbs2.put(adbs);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_DBS_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.dbs2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL bIsZHDRTuning = m_pIspTuningCustom->is_ZHDRTuning(rIspCamInfo);

    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgDbs2En = fgPureRaw && fgPgnEn;
    ISP_MGR_DBS2_T& rDbs2 = ISP_MGR_DBS2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgDbs2En|| bDisable || !(bIsZHDRTuning) || (!rIspCamInfo.fgCCTInfo.Bits.fgDBS)||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_DBS_EN)))
    {
        rDbs2.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rDbs2.setEnable(MTRUE);

        // Get default NVRAM parameter
        RAWIspCamInfo temp_CamInfo = rIspCamInfo;
        getISPIndex(eIDX_LowerIso_LowerZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs2.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_DBS_T dbs = m_rIspParam.ISPRegs.DBS[m_ISP_INT.sDbs2.u2LowerIso_LowerZoom];

        // HDR Flow Setting
        temp_CamInfo.rAEInfo.u4RealISOValue = m_pIspTuningCustom->get_HdrDbsNewISO(rIspCamInfo);
        temp_CamInfo.rMapping_Info.eISO_Idx = map_ISO_value2index(temp_CamInfo.rAEInfo.u4RealISOValue);
        get_ISOIdx_neighbor(temp_CamInfo, temp_CamInfo.rMapping_Info.eISO_Idx, temp_CamInfo.rAEInfo.u4RealISOValue);

        //Interpolation ISP module
        if (isIspInterpolation(temp_CamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs2.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs2.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , temp_CamInfo, EModule_DBS, m_ISP_INT.sDbs2.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): customized DBS_Iso = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, temp_CamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sDbs2.u2UpperIso_UpperZoom,
                     m_ISP_INT.sDbs2.u2LowerIso_UpperZoom,
                     m_ISP_INT.sDbs2.u2UpperIso_LowerZoom,
                     m_ISP_INT.sDbs2.u2LowerIso_LowerZoom);

            SmoothDBS(temp_CamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.DBS[m_ISP_INT.sDbs2.u2UpperIso_LowerZoom], // DBS settings for upper ISO
                    m_rIspParam.ISPRegs.DBS[m_ISP_INT.sDbs2.u2LowerIso_LowerZoom], // DBS settings for lower ISO
                    dbs);  // Output

            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom, temp_CamInfo, EModule_DBS, m_ParamIdx_P2.DBS, m_bMappingQueryFlag);
            dbs = m_rIspParam.ISPRegs.DBS[m_ParamIdx_P2.DBS];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_DBS(rIspCamInfo, m_IspNvramMgr, dbs);
        }

        // Load it to ISP manager buffer.
        rDbs2.put(dbs);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_OBC_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.obc2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgObc2En = fgPureRaw && fgPgnEn;
    ISP_MGR_OBC2_T& rObc2 = ISP_MGR_OBC2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if ( !fgObc2En || bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgOBC)||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_OBC_EN)))
    {
        rObc2.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rObc2.setEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_OBC, m_ISP_INT.sObc2.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_OBC_T obc = m_rIspParam.ISPRegs.OBC[m_ISP_INT.sObc2.u2LowerIso_LowerZoom];

        rObc2.setIspAEGain(rIspCamInfo.rAEInfo.u4IspGain>>1);

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_OBC, m_ISP_INT.sObc2.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_OBC, m_ISP_INT.sObc2.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_OBC, m_ISP_INT.sObc2.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sObc2.u2UpperIso_UpperZoom,
                     m_ISP_INT.sObc2.u2LowerIso_UpperZoom,
                     m_ISP_INT.sObc2.u2UpperIso_LowerZoom,
                     m_ISP_INT.sObc2.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothOBC_2);
            SmoothOBC(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.OBC[m_ISP_INT.sObc2.u2UpperIso_LowerZoom], // OBC settings for upper ISO
                    m_rIspParam.ISPRegs.OBC[m_ISP_INT.sObc2.u2LowerIso_LowerZoom], // OBC settings for lower ISO
                    obc);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_OBC, m_ParamIdx_P2.OBC, m_bMappingQueryFlag);
            obc = m_rIspParam.ISPRegs.OBC[m_ParamIdx_P2.OBC];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_OBC(rIspCamInfo, m_IspNvramMgr, obc);
        }

        this->setPureOBCInfo(&obc);

        // Load it to ISP manager buffer.
        rObc2.put(obc);
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_BPC_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.bpc2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgBpc2En = fgPureRaw && fgPgnEn;
    ISP_MGR_BNR2_T& rBnr2 = ISP_MGR_BNR2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgBpc2En|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgBPC)||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_BPC_EN)))
    {
        rBnr2.setBPC2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setBPC2Enable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc2.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_BNR_BPC_T bpc = m_rIspParam.ISPRegs.BNR_BPC[m_ISP_INT.sBpc2.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc2.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc2.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_BNR_BPC, m_ISP_INT.sBpc2.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sBpc2.u2UpperIso_UpperZoom,
                     m_ISP_INT.sBpc2.u2LowerIso_UpperZoom,
                     m_ISP_INT.sBpc2.u2UpperIso_LowerZoom,
                     m_ISP_INT.sBpc2.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothBPC_2);
            SmoothBPC(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.BNR_BPC[m_ISP_INT.sBpc2.u2UpperIso_LowerZoom], // BPC settings for upper ISO
                    m_rIspParam.ISPRegs.BNR_BPC[m_ISP_INT.sBpc2.u2LowerIso_LowerZoom], // BPC settings for lower ISO
                    bpc);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_BNR_BPC, m_ParamIdx_P2.BNR_BPC, m_bMappingQueryFlag);
            bpc = m_rIspParam.ISPRegs.BNR_BPC[m_ParamIdx_P2.BNR_BPC];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BPC(rIspCamInfo, m_IspNvramMgr, bpc);
        }

        // Load it to ISP manager buffer.
        rBnr2.put(bpc);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR1_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.nr12.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgNr12En = fgPureRaw && fgPgnEn;
    ISP_MGR_BNR2_T& rBnr2 = ISP_MGR_BNR2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if ( !fgNr12En || bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgNR1) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_NR1_EN)))
    {
        rBnr2.setCT2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setCT2Enable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr12.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_BNR_NR1_T nr1 = m_rIspParam.ISPRegs.BNR_NR1[m_ISP_INT.sNr12.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr12.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr12.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_BNR_NR1, m_ISP_INT.sNr12.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                        __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                         m_ISP_INT.sNr12.u2UpperIso_UpperZoom,
                         m_ISP_INT.sNr12.u2LowerIso_UpperZoom,
                         m_ISP_INT.sNr12.u2UpperIso_LowerZoom,
                         m_ISP_INT.sNr12.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothNR1_2);
            SmoothNR1(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.BNR_NR1[m_ISP_INT.sNr12.u2UpperIso_LowerZoom], // NR1 settings for upper ISO
                    m_rIspParam.ISPRegs.BNR_NR1[m_ISP_INT.sNr12.u2LowerIso_LowerZoom], // NR1 settings for lower ISO
                    nr1);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_BNR_NR1, m_ParamIdx_P2.BNR_NR1, m_bMappingQueryFlag);
            nr1 = m_rIspParam.ISPRegs.BNR_NR1[m_ParamIdx_P2.BNR_NR1];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_NR1(rIspCamInfo, m_IspNvramMgr, nr1);
        }

        // Load it to ISP manager buffer.
        rBnr2.put(nr1);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PDC_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.pdc2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgPdc2En = fgPureRaw && fgPgnEn;
    ISP_MGR_BNR2_T& rBnr2 = ISP_MGR_BNR2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgPdc2En|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgPDC)||
        (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_YUV_Reprocess))
    {
        rBnr2.setPDC2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setPDC2Enable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc2.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_BNR_PDC_T pdc = m_rIspParam.ISPRegs.BNR_PDC[m_ISP_INT.sPdc2.u2UpperIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc2.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc2.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_BNR_PDC, m_ISP_INT.sPdc2.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                     m_ISP_INT.sPdc2.u2UpperIso_UpperZoom,
                     m_ISP_INT.sPdc2.u2LowerIso_UpperZoom,
                     m_ISP_INT.sPdc2.u2UpperIso_LowerZoom,
                     m_ISP_INT.sPdc2.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothPDC_2);
            SmoothPDC(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.BNR_PDC[m_ISP_INT.sPdc2.u2UpperIso_LowerZoom], // PDC settings for upper ISO
                    m_rIspParam.ISPRegs.BNR_PDC[m_ISP_INT.sPdc2.u2LowerIso_LowerZoom], // PDC settings for lower ISO
                    pdc);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_BNR_PDC, m_ParamIdx_P2.BNR_PDC, m_bMappingQueryFlag);
            pdc = m_rIspParam.ISPRegs.BNR_PDC[m_ParamIdx_P2.BNR_PDC];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_PDC(rIspCamInfo, m_IspNvramMgr, pdc);
        }

        // Load it to ISP manager buffer.
        rBnr2.put(pdc);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RMM_2(const RAWIspCamInfo& rIspCamInfo)
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmm2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgRmm2En = fgPureRaw && fgPgnEn;
    ISP_MGR_RMM2_T& rRmm2 = ISP_MGR_RMM2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgRmm2En || bDisable || (!(rIspCamInfo.fgCCTInfo.Bits.fgRMM)) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_RMM_EN)))
    {
        rRmm2.setEnable(MFALSE);
    }
    else
    {
        rRmm2.setEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm2.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_RMM_T rmm = m_rIspParam.ISPRegs.RMM[m_ISP_INT.sRmm2.u2LowerIso_LowerZoom];

        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm2.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm2.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_RMM, m_ISP_INT.sRmm2.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                        __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                        m_ISP_INT.sRmm2.u2UpperIso_UpperZoom,
                        m_ISP_INT.sRmm2.u2LowerIso_UpperZoom,
                        m_ISP_INT.sRmm2.u2UpperIso_LowerZoom,
                        m_ISP_INT.sRmm2.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothRMM_2);
            SmoothRMM(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                       m_rIspParam.ISPRegs.RMM[m_ISP_INT.sRmm2.u2UpperIso_LowerZoom], // RMM settings for upper ISO
                       m_rIspParam.ISPRegs.RMM[m_ISP_INT.sRmm2.u2LowerIso_LowerZoom], // RMM settings for lower ISO
                       rmm);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_RMM, m_ParamIdx_P2.RMM, m_bMappingQueryFlag);
            rmm = m_rIspParam.ISPRegs.RMM[m_ParamIdx_P2.RMM];
        }

        // Invoke callback for customers to modify.
        if ( isDynamicTuning() )
        {
            //Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_RMM(rIspCamInfo, m_IspNvramMgr, rmm);
        }

        // Load it to ISP manager buffer.
        rRmm2.put(rmm);
    }
#endif
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RMG_2(const RAWIspCamInfo& rIspCamInfo)
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmg2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgRmg2En = fgPureRaw && fgPgnEn;
    ISP_MGR_RMG2_T& rRmg2 = ISP_MGR_RMG2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgRmg2En || bDisable  ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_RMG_EN)))
    {
        rRmg2.setEnable(MFALSE);
        }
    else
    {
        rRmg2.setEnable(MTRUE);
    }
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CPN_2(const RAWIspCamInfo& rIspCamInfo)
{
    ISP_MGR_CPN2_T& rCpn2 = ISP_MGR_CPN2_T::getInstance(m_eSensorDev);

    rCpn2.setEnable(MTRUE);

    if(rCpn2.isEnable() && (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_RMG_EN)) {
        rCpn2.setZHdrEnable(MTRUE);
    }
    else {
        rCpn2.setZHdrEnable(MFALSE); //bypass mode
    }

    //Force set ZHdrEnable
    MINT32 forceZhdrEn = ::property_get_int32("vendor.debug.forceset.isp.zhdr.enable", -1);
    if(forceZhdrEn != -1)
    {
        if(forceZhdrEn == 0)
            rCpn2.setZHdrEnable(MFALSE); //bypass mode
        if(forceZhdrEn == 1)
            rCpn2.setZHdrEnable(MTRUE);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_DCPN_2(const RAWIspCamInfo& rIspCamInfo)
{
    ISP_MGR_DCPN2_T& rDcpn2 = ISP_MGR_DCPN2_T::getInstance(m_eSensorDev);

    rDcpn2.setEnable(MTRUE);

    if(rDcpn2.isEnable() && (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_RMG_EN)) {
        rDcpn2.setZHdrEnable(MTRUE);
    }
    else {
        rDcpn2.setZHdrEnable(MFALSE); //bypass mode
    }

    //Force set ZHdrEnable
    MINT32 forceZhdrEn = ::property_get_int32("vendor.debug.forceset.isp.zhdr.enable", -1);
    if(forceZhdrEn != -1)
    {
        if(forceZhdrEn == 0)
            rDcpn2.setZHdrEnable(MFALSE); //bypass mode
        if(forceZhdrEn == 1)
            rDcpn2.setZHdrEnable(MTRUE);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC_2(const ISP_INFO_T& rIspInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.lsc2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL fgRet = MFALSE;
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspInfo.rCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspInfo.rCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    EIspProfile_T profile = rIspInfo.rCamInfo.rMapping_Info.eIspProfile;
    CAM_LOGD_IF((rIspInfo.rLscData.size()>0), "[%s] Profile = %d, FMT = %d", __FUNCTION__, profile, rIspInfo.rCamInfo.i4P2InImgFmt);
    MBOOL fgLsc2En = fgPureRaw && fgPgnEn;
    ISP_MGR_LSC2_T& rLsc2 = ISP_MGR_LSC2_T::getInstance(m_eSensorDev);

    if (fgLsc2En && rIspInfo.rLscData.size())
    {
        ILscTable::Config rCfg;
        ILscTbl::RsvdData rRsvd;
        ::memcpy(&rCfg, rIspInfo.rLscData.data(), sizeof(ILscTable::Config));
        ::memcpy(&rRsvd, rIspInfo.rLscData.data()+sizeof(ILscTable::Config), sizeof(ILscTbl::RsvdData));
        if (rCfg.i4GridX > 17 || rCfg.i4GridY > 17 || rCfg.i4GridX <= 0 || rCfg.i4GridY <= 0 || rCfg.i4ImgWd <= 0 || rCfg.i4ImgHt <= 0 || rRsvd.u4HwRto > 32)
        {
            CAM_LOGE("Abnormal config (%d,%d,%d,%d,%d)", rCfg.i4GridX, rCfg.i4GridY, rCfg.i4ImgWd, rCfg.i4ImgHt, rRsvd.u4HwRto);
        }
        else
        {
            MUINT32 u4W, u4H;
            if (m_pLscMgr->getRawSize((ESensorMode_T) rIspInfo.rCamInfo.rMapping_Info.eSensorMode, u4W, u4H))
            {
                CAM_LOGD("[%s] Process Lsc Data @(%d), Modify Size(%dx%d -> %dx%d), grid(%dx%d), HwRto(%d)", __FUNCTION__,
                    rIspInfo.rCamInfo.u4Id, rCfg.i4ImgWd, rCfg.i4ImgHt, u4W, u4H, rCfg.i4GridX, rCfg.i4GridY, rRsvd.u4HwRto);
                #if CAM3_LSC2_USE_GAIN
                ILscTbl rTbl(ILscTable::GAIN_FIXED, u4W, u4H, rCfg.i4GridX, rCfg.i4GridY);
                #else
                ILscTbl rTbl(ILscTable::HWTBL, u4W, u4H, rCfg.i4GridX, rCfg.i4GridY);
                #endif
                //if (profile == EIspProfile_N3D_Denoise_toW)
                //    rTbl.setRsvdData(ILscTbl::RsvdData(32));
                //else
                    rTbl.setRsvdData(rRsvd);
                rTbl.setData(rIspInfo.rLscData.data()+sizeof(ILscTable::Config)+sizeof(ILscTbl::RsvdData), rTbl.getSize());
                m_pLscMgr->syncTbl(rTbl, ILscMgr::LSC_P2);

                MINT32 captureDump = property_get_int32("vendor.debug.camera.dump.lsc2", 0);
                MINT32 previewDump = property_get_int32("vendor.debug.camera.dump.lsc2.preview", 0);

                FileReadRule rule;
                MBOOL bdumplsc = rule.isDumpEnable("3AHAL")&&
                ((rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_After_Blend) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Single) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Before_Blend));

                if (rIspInfo.isCapture && captureDump || previewDump)
                {
                    char strlscdump[512] = {'\0'};
                    genFileName_LSC2(strlscdump, sizeof(strlscdump), &rIspInfo.hint);

                    sp<IFileCache> fidLscDump;
                    fidLscDump = IFileCache::open(strlscdump);
                    if (fidLscDump->write(&rTbl.getConfig(), sizeof(ILscTable::Config)) != sizeof(ILscTable::Config))
                        CAM_LOGW("write config error to %s", strlscdump);
                    if (fidLscDump->write(&rTbl.getRsvdData().u4HwRto, sizeof(MUINT32)) != sizeof(MUINT32))
                        CAM_LOGW("write rsvdata error to %s", strlscdump);
                    if (fidLscDump->write(rIspInfo.rLscData.data()+sizeof(ILscTable::Config)+sizeof(MUINT32), rTbl.getSize()) != rTbl.getSize())
                        CAM_LOGW("write shading table error to %s", strlscdump);
                }

                fgRet = MTRUE;
            }
            else
            {
                CAM_LOGE("Fail to get RAW size!");
            }
        }
        //fgLsc2En = (profile != EIspProfile_N3D_Denoise) && (profile != EIspProfile_N3D_Denoise_toGGM);
        //:2185CAM_LOGD_IF(m_bDebugEnable, "[%s] fgLsc2En = %d", __FUNCTION__, fgLsc2En);
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] force off(%d), lsc2(%d), pgn(%d), pureraw(%d), fgRet(%d)", __FUNCTION__, bDisable, fgLsc2En, fgPgnEn, fgPureRaw, fgRet);

    if (!fgLsc2En || bDisable || (fgRet == MFALSE)||
        (rIspInfo.rCamInfo.i4P2InImgFmt) ||
        (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == EIspProfile_YUV_Reprocess) ||
        (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Capture_MultiPass_HWNR))
    {
        rLsc2.enableLsc(MFALSE);
    }
    else
    {
        CAM_LOGD("[%s] force off(%d), lsc2(%d), pgn(%d), pureraw(%d), fgRet(%d)", __FUNCTION__, bDisable, fgLsc2En, fgPgnEn, fgPureRaw, fgRet);
        rLsc2.enableLsc(MTRUE);
    }

    return MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_SL2(const ISP_INFO_T& rIspInfo)
{
    ISP_MGR_SL2_T& rSl2 = ISP_MGR_SL2_T::getInstance(m_eSensorDev);
    const NSIspTuning::RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;

//SL2, UDM
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2.disable", value, "0"); // 0: enable, 1: disable
    MUINT32 bDisable = atoi(value);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2_EN)))
    {
        rSl2.setEnable(MFALSE);
    }
    else
    {
        rSl2.setEnable(MTRUE);
    }

//SL2B, NBC
    char valueB[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2b.disable", valueB, "0"); // 0: enable, 1: disable
    MUINT32 bDisableSL2B = atoi(valueB);

    if ((bDisableSL2B)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2B_EN)))
    {
        rSl2.setSL2BEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2BEnable(MTRUE);
    }

//SL2C, NBC2
    char valueC[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2c.disable", valueC, "0"); // 0: enable, 1: disable
    MUINT32 bDisableSL2C = atoi(valueC);

    if ((bDisableSL2C)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2C_EN)))
    {
        rSl2.setSL2CEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2CEnable(MTRUE);
    }

//SL2D, SEEE
    char valueD[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2d.disable", valueD, "0"); // 0: enable, 1: disable
    MUINT32 bDisableSL2D = atoi(valueD);

    if ((bDisableSL2D)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2D_EN)))
    {
        rSl2.setSL2DEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2DEnable(MTRUE);
    }

//SL2E, NR3D
    char valueE[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2e.disable", valueE, "0"); // 0: enable, 1: disable
    MUINT32 bDisableSL2E = atoi(valueE);

    if ((bDisableSL2E)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2E_EN)))
    {
        rSl2.setSL2EEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2EEnable(MTRUE);
    }

//SL2H, RNR
    char valueH[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2h.disable", valueH, "0"); // 0: enable, 1: disable
    MUINT32 bDisableSL2H = atoi(valueH);

    if ((bDisableSL2H)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2H_EN)))
    {
        rSl2.setSL2HEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2HEnable(MTRUE);
    }

//SL2I, HFG
    char valueI[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2i.disable", valueI, "0"); // 0: enable, 1: disable
    MUINT32 bDisableSL2I = atoi(valueI);

    if ((bDisableSL2I)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SL2I_EN)))
    {
        rSl2.setSL2IEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2IEnable(MTRUE);
    }

    MINT32 dbg_enable = ::property_get_int32("vendor.debug.lsc_mgr.nsl2", -1);
    MINT32 i4Sl2Case = (dbg_enable != -1) ? dbg_enable : isEnableSL2(m_eSensorDev);

    getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo, EModule_SL2, m_ParamIdx_P2.SL2, m_bMappingQueryFlag);
    CAM_LOGD_IF(m_bDebugEnable,"[prepareHw_PerFrame_SL2] getISPIndex(%d), profile(%d)", m_ParamIdx_P2.SL2, rIspCamInfo.rMapping_Info.eIspProfile);

    if ( rSl2.isEnable()     || rSl2.getSL2BOnOff() || rSl2.getSL2COnOff() || rSl2.getSL2DOnOff() ||
         rSl2.getSL2EOnOff() || rSl2.getSL2HOnOff() || rSl2.getSL2IOnOff())
    {
#if 0
        switch (rIspCamInfo.rMapping_Info.eIspProfile)
        {
            case EIspProfile_MFNR_Single:
            case EIspProfile_MFNR_After_Blend:
                m_IspNvramMgr.setIdx_SL2(40);
                m_ParamIdx_P2.SL2 = 40;
                break;
            case EIspProfile_MFNR_Single_ZOOM1:
            case EIspProfile_MFNR_After_ZOOM1:
                m_IspNvramMgr.setIdx_SL2(41);
                m_ParamIdx_P2.SL2 = 41;
                break;
            case EIspProfile_MFNR_Single_ZOOM2:
            case EIspProfile_MFNR_After_ZOOM2:
                m_IspNvramMgr.setIdx_SL2(42);
                m_ParamIdx_P2.SL2 = 42;
                break;
            case EIspProfile_N3D_Preview:
            case EIspProfile_N3D_Preview_toW:
                m_IspNvramMgr.setIdx_SL2(43);
                m_ParamIdx_P2.SL2 = 43;
                break;
            case EIspProfile_N3D_Video:
            case EIspProfile_N3D_Video_toW:
                m_IspNvramMgr.setIdx_SL2(44);
                m_ParamIdx_P2.SL2 = 44;
                break;
            case EIspProfile_N3D_Capture:
            case EIspProfile_N3D_Capture_toW:
                m_IspNvramMgr.setIdx_SL2(45);
                m_ParamIdx_P2.SL2 = 45;
                break;
            //case EIspProfile_N3D_Denoise_toYUV:
            //case EIspProfile_N3D_Denoise_toW:
            //    m_IspNvramMgr.setIdx_SL2(46);
            //    m_ParamIdx_P2.SL2 = 46;
            //    break;
/*            case EIspProfile_N3D_HighRes_toYUV:
                m_IspNvramMgr.setIdx_SL2(47);
                m_ParamIdx_P2.SL2 = 47;
                break;*/
            //case EIspProfile_N3D_MFHR_Before_Blend:
            ///    m_IspNvramMgr.setIdx_SL2(48);
            //    m_ParamIdx_P2.SL2 = 48;
            //    break;
            //case EIspProfile_N3D_MFHR_Single:
            //case EIspProfile_N3D_MFHR_After_Blend:
            case EIspProfile_zHDR_Capture_MFNR_Single:
            case EIspProfile_zHDR_Capture_MFNR_After_Blend:
                m_IspNvramMgr.setIdx_SL2(49);
                m_ParamIdx_P2.SL2 = 49;
                break;
            default:
                m_IspNvramMgr.setIdx_SL2(
                    rIspCamInfo.rMapping_Info.eSensorMode*SHADING_SUPPORT_CT_NUM+ (UINT16)m_pIspTuningCustom->evaluate_Shading_CCT_index(rIspCamInfo));
                m_ParamIdx_P2.SL2 = (
                    rIspCamInfo.rMapping_Info.eSensorMode*SHADING_SUPPORT_CT_NUM+ (UINT16)m_pIspTuningCustom->evaluate_Shading_CCT_index(rIspCamInfo));
        }
#endif
        // Get default NVRAM parameter
        ISP_NVRAM_SL2_T sl2 = m_rIspParam.ISPRegs.SL2[m_ParamIdx_P2.SL2];

        if ( i4Sl2Case == 1)
        {
            // use LSC determined SL2
            sl2.cen.bits.SL2_CENTR_X = rIspCamInfo.rSl2Info.i4CenterX;
            sl2.cen.bits.SL2_CENTR_Y = rIspCamInfo.rSl2Info.i4CenterY;
            sl2.rr_con0.bits.SL2_R_0 = rIspCamInfo.rSl2Info.i4R0;
            sl2.rr_con0.bits.SL2_R_1 = rIspCamInfo.rSl2Info.i4R1;
            sl2.rr_con1.bits.SL2_R_2 = rIspCamInfo.rSl2Info.i4R2;
        }

        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_SL2(rIspCamInfo, m_IspNvramMgr, sl2);
        }

        rSl2.put(sl2);

    }


    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.pgn.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEnable = (!rIspCamInfo.fgRPGEnable);
    ISP_MGR_PGN_T& rPgn = ISP_MGR_PGN_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    AWB_GAIN_T rCurrentAWBGain = rIspCamInfo.rAWBInfo.rPGN;
    MINT16 i2FlareOffset = rIspCamInfo.rAEInfo.i2FlareOffset;
    if(m_pIspTuningCustom->is_to_invoke_flc(rIspCamInfo)){
        i2FlareOffset = 0;
    }

    MINT16 i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - i2FlareOffset);

    CAM_LOGD_IF(m_bDebugEnable, "[%s] En(%d), AWB(%d,%d,%d), AE(%d,%d) \n", __FUNCTION__,
            fgPgnEnable, rCurrentAWBGain.i4R, rCurrentAWBGain.i4G, rCurrentAWBGain.i4B,i2FlareOffset,i2FlareGain );

    rPgn.setIspAWBGain(rCurrentAWBGain);
    rPgn.setIspFlare(i2FlareGain, (-1*i2FlareOffset));
    if (!fgPgnEnable || (bDisable)||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_RPGN_EN))) {// RPG is enabled
        rPgn.setEnable(MFALSE);
    }
    else
    {
        rPgn.setEnable(MTRUE);

        getIspHWBuf(m_eSensorDev, pgn );

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_PGN(rIspCamInfo, m_IspNvramMgr, pgn);
        }

        // Load it to ISP manager buffer.
        rPgn.put(pgn);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RNR(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rnr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_RNR_T& rRnr = ISP_MGR_RNR_T::getInstance(m_eSensorDev);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||(!rRnr.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_RNR_EN)))
    {
        rRnr.setEnable(MFALSE);
    }
    else{
        rRnr.setEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_RNR, m_ISP_INT.sRnr.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_RNR_T rnr = m_rIspParam.ISPRegs.RNR[m_ISP_INT.sRnr.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_RNR, m_ISP_INT.sRnr.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_RNR, m_ISP_INT.sRnr.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_RNR, m_ISP_INT.sRnr.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                        __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                        m_ISP_INT.sRnr.u2UpperIso_UpperZoom,
                        m_ISP_INT.sRnr.u2LowerIso_UpperZoom,
                        m_ISP_INT.sRnr.u2UpperIso_LowerZoom,
                        m_ISP_INT.sRnr.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothRNR);
            SmoothRNR(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                       m_rIspParam.ISPRegs.RNR[m_ISP_INT.sRnr.u2UpperIso_LowerZoom], // RNR settings for upper ISO
                       m_rIspParam.ISPRegs.RNR[m_ISP_INT.sRnr.u2LowerIso_LowerZoom], // RNR settings for lower ISO
                       rnr);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_RNR, m_ParamIdx_P2.RNR, m_bMappingQueryFlag);
            rnr = m_rIspParam.ISPRegs.RNR[m_ParamIdx_P2.RNR];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_RNR(rIspCamInfo, m_IspNvramMgr, rnr);
        }

        // FG mode protection
        if (rIspCamInfo.fgRPGEnable){
            rnr.con3.bits.RNR_FL_MODE = 1;
        }
        else{
            rnr.con3.bits.RNR_FL_MODE = 0;
        }

        // Load it to ISP manager buffer.
        rRnr.put(rnr);
    }
    return  MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_UDM(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.udm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    ISP_MGR_UDM_T& rUdm = ISP_MGR_UDM_T::getInstance(m_eSensorDev);

    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    // Get default NVRAM parameter
    getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_UDM, m_ISP_INT.sUdm.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
    ISP_NVRAM_UDM_T udm = m_rIspParam.ISPRegs.UDM[m_ISP_INT.sUdm.u2LowerIso_LowerZoom];

if(rIspCamInfo.i4P2InImgFmt == 0){
    //Interpolation ISP module
    if (isIspInterpolation(rIspCamInfo))
    {
        getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_UDM, m_ISP_INT.sUdm.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
        getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_UDM, m_ISP_INT.sUdm.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
        getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_UDM, m_ISP_INT.sUdm.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

        m_ParamIdx_P2.UDM = m_ISP_INT.sUdm.u2LowerIso_LowerZoom;
        CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                    m_ISP_INT.sUdm.u2UpperIso_UpperZoom,
                    m_ISP_INT.sUdm.u2LowerIso_UpperZoom,
                    m_ISP_INT.sUdm.u2UpperIso_LowerZoom,
                    m_ISP_INT.sUdm.u2LowerIso_LowerZoom);

        ISP_NVRAM_UDM_INT_T rInt_Param;
        rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
        rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
        rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
        rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
        rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
        rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
        rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.UDM[m_ISP_INT.sUdm.u2UpperIso_UpperZoom];
        rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.UDM[m_ISP_INT.sUdm.u2LowerIso_UpperZoom];
        rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.UDM[m_ISP_INT.sUdm.u2UpperIso_LowerZoom];
        rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.UDM[m_ISP_INT.sUdm.u2LowerIso_LowerZoom];

        AAA_TRACE_ALG(SmoothUDM);
        SmoothUDM(rInt_Param, udm);
        AAA_TRACE_END_ALG;
    }
    else{
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_UDM, m_ParamIdx_P2.UDM, m_bMappingQueryFlag);
        udm = m_rIspParam.ISPRegs.UDM[m_ParamIdx_P2.UDM];
    }

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_UDM(rIspCamInfo, m_IspNvramMgr, udm);
    }

    // FG mode protection
    if (fgRPGEnable)
    {
        udm.dsb.bits.UDM_FL_MODE = 1;
        udm.dsb.bits.UDM_SL_RAT = 16;
        udm.dsb.bits.UDM_SC_RAT = 16;
    }
    else {
        udm.dsb.bits.UDM_FL_MODE = 0;
    }

    if ((!rUdm.isCCTEnable()) || (bDisable) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_UDM_EN))) // bypass UDM    //(m_u4RawFmtType == SENSOR_RAW_MONO)?
    {
        udm.intp_crs.bits.UDM_BYP = 1;
    }

    // Load it to ISP manager buffer.
    rUdm.put(udm);

}
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_FLC(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.flc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_MGR_FLC_T& rFlc = ISP_MGR_FLC_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    ISP_NVRAM_FLC_T flc;

    MINT16 i2FlareOffset = rIspCamInfo.rAEInfo.i2FlareOffset;

    MINT16 i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - i2FlareOffset);

    rFlc.setFLC(i2FlareGain, (-1*i2FlareOffset));

    if ((!m_pIspTuningCustom->is_to_invoke_flc(rIspCamInfo)) ||
        (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE) ||
        bDisable) {// RPG is enabled
        rFlc.setEnable(MFALSE);
    }
    else
    {
        rFlc.setEnable(MTRUE);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    property_get("vendor.isp.ccm.supportmute", value, "0");
    MBOOL bSupportMute = atoi(value);
    MBOOL bManualCCM = (rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX);

    MBOOL fgBlackMute = rIspCamInfo.fgBlackMute && bSupportMute;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] Mute(%d), manual CCM %d", __FUNCTION__, fgBlackMute, bManualCCM);

    if ((bDisable) ||
        (!ISP_MGR_CCM_T::getInstance(m_eSensorDev).isCCTEnable())            ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_CCM_EN))) {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        ISP_NVRAM_CCM_T rCCM = rIspCamInfo.rMtkCCM;

        if(((rIspCamInfo.eColorCorrectionMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) && (!rIspCamInfo.rAWBInfo.bAWBLock)) ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_toW)  ||
            //(rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Denoise_toW)  ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW)  ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Video_toW) ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Depth_toW))
        {
            // Dynamic CCM
            if (isDynamicCCM())
            {
#if MTK_CAM_NEW_NVRAM_SUPPORT
                getISPIndex(eIDX_LowerLV, rIspCamInfo, EModule_CCM, m_ISP_INT.sCCM2.u2LowerLv_LowerCt, m_bMappingQueryFlag);
                getISPIndex(eIDX_UpperLV, rIspCamInfo, EModule_CCM, m_ISP_INT.sCCM2.u2UpperLv_LowerCt, m_bMappingQueryFlag);

                MY_LOG_IF(m_bDebugEnable, "Smooth CCM");
                CAM_LOGD_IF(m_bDebugEnable,"%s(): i4RealLightValue_x10 = %d, u2LowerLv = %d, u2UpperLv = %d\n",
                           __FUNCTION__, rIspCamInfo.rAEInfo.i4RealLightValue_x10,
                           m_ISP_INT.sCCM2.u2LowerLv_LowerCt,
                           m_ISP_INT.sCCM2.u2UpperLv_LowerCt);

                AAA_TRACE_ALG(calculateCCM);

                m_pCcmMgr->calculateCCM(rCCM, m_ISP_ALG_VER.sCCM,
                    rIspCamInfo,
                    m_rIspParam.ISPColorTbl.CCM[m_ISP_INT.sCCM2.u2UpperLv_LowerCt],
                    m_rIspParam.ISPColorTbl.CCM[m_ISP_INT.sCCM2.u2LowerLv_LowerCt],
                    m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
                    m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L]);

                AAA_TRACE_END_ALG;
#endif
                //transfer CCM data to CCM2
                ISP_MGR_CCM2_T::getInstance(m_eSensorDev).put(rCCM);

                // Invoke callback for customers to modify.
                if  ( isDynamicTuning() )
                {   //  Dynamic Tuning: Enable
                    m_pIspTuningCustom->refine_CCM(rIspCamInfo, m_IspNvramMgr, rCCM);
                }
            }
        }

        if (fgBlackMute)
        {
            ::memset(&rCCM, 0, sizeof(ISP_NVRAM_CCM_T));
        }

        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setManual(bManualCCM);

        // Load it to ISP manager buffer.
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).put(rCCM);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccm2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    property_get("vendor.isp.ccm2.supportmute", value, "0");
    MBOOL bSupportMute = atoi(value);

    MBOOL fgBlackMute = rIspCamInfo.fgBlackMute && bSupportMute;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] Mute(%d)", __FUNCTION__, fgBlackMute);

    if ((bDisable) ||
        (!ISP_MGR_CCM2_T::getInstance(m_eSensorDev).isCCTEnable())            ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_CCM2_EN))) {
        ISP_MGR_CCM2_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_CCM2_T::getInstance(m_eSensorDev).setEnable(MTRUE);


        ISP_NVRAM_CCM_T rCCM = rIspCamInfo.rMtkCCM;;
        if(((rIspCamInfo.eColorCorrectionMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) && (!rIspCamInfo.rAWBInfo.bAWBLock)) ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_toW)  ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview)  ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW)  ||
            (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Video_toW))
        {
            if (isDynamicCCM())
            {
                ISP_MGR_CCM2_T::getInstance(m_eSensorDev).get(rCCM);

                // Invoke callback for customers to modify.
                if  ( isDynamicTuning() )
                {   //  Dynamic Tuning: Enable
                    m_pIspTuningCustom->refine_CCM2(rIspCamInfo, m_IspNvramMgr, rCCM);
                }
            }
        }

        if (fgBlackMute)
        {
            ::memset(&rCCM, 0, sizeof(ISP_NVRAM_CCM_T));
        }

        // Load it to ISP manager buffer.
        ISP_MGR_CCM2_T::getInstance(m_eSensorDev).put(rCCM);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ggm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_NVRAM_GGM_T ggm;

    if(rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE){
        ggm = m_pGgmMgr->getGGM();
    }
    else
    {
        ggm.lut = m_rIspParam.ISPToneMap.GMA[0].GGM_Reg[0];

        if(rIspCamInfo.u1P2TuningUpdate == 5){

            RAWIspCamInfo rIspCamInfo_ = rIspCamInfo;
            rIspCamInfo_.rMapping_Info.eIspProfile = EIspProfile_Preview;
            getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo_, EModule_GMA, m_ISP_INT.u2GGM, m_bMappingQueryFlag);
        }
        else{
            getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo, EModule_GMA, m_ISP_INT.u2GGM, m_bMappingQueryFlag);
        }
        AAA_TRACE_ALG(calculateGGM);
        m_pGmaMgr->calculateGGM(&ggm, &m_GmaExifInfo,
                                m_bSlave, m_ISP_Sync_Info,
                                rIspCamInfo.rAEInfo, m_ISP_INT.u2GGM);
        AAA_TRACE_END_ALG;

        // Invoke callback for customers to modify.
        if ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_GGM(rIspCamInfo,  m_IspNvramMgr, ggm);
            m_pIspTuningCustom->userSetting_EFFECT_GGM(rIspCamInfo, rIspCamInfo.eIdx_Effect, ggm);
        }
        m_pGgmMgr->updateGGM(ggm);
    }

    ISP_MGR_GGM_T& rGgm = ISP_MGR_GGM_T::getInstance(m_eSensorDev);

    if ( (!rGgm.isCCTEnable()) || (bDisable)  ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_GGM_EN))) // GGM_LNR = 1
    {
        rGgm.setLNREnable(MTRUE);
    }
    else
    {
        rGgm.setLNREnable(MFALSE);

        // Load it to ISP manager buffer.
        rGgm.put(ggm);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM_2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ggm2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_NVRAM_GGM_T ggm;
    ISP_MGR_GGM_T::getInstance(m_eSensorDev).get(ggm);

    ISP_MGR_GGM2_T& rGgm2 = ISP_MGR_GGM2_T::getInstance(m_eSensorDev);

    if ( (!rGgm2.isCCTEnable()) || (bDisable)  ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_GGM2_EN))) // GGM_LNR = 1
    {
        rGgm2.setLNREnable(MTRUE);
    }
    else
    {
        rGgm2.setLNREnable(MFALSE);

        // Load it to ISP manager buffer.
        rGgm2.put(ggm);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NDG(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ndg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_NDG_T& rNdg = ISP_MGR_NDG_T::getInstance(m_eSensorDev);

    if ((!rNdg.isCCTEnable()) || (bDisable) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_NDG_EN)))
    {
        rNdg.setEnable(MFALSE);
    }
    else{
        rNdg.setEnable(MTRUE);
    }



    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.anr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    ISP_MGR_NBC_T& rNbc = ISP_MGR_NBC_T::getInstance(m_eSensorDev);


    if ((bDisable) || (!rNbc.isCCTANR1Enable()) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_ANR_EN) ||
        (rIspCamInfo.bBypassNR))
    )
    {
        rNbc.setANR1Enable(MFALSE);
    }
    else {

        rNbc.setANR1Enable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC_ANR, m_ISP_INT.sAnr.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_ANR_T anr = m_rIspParam.ISPRegs.NBC_ANR[m_ISP_INT.sAnr.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_NBC_ANR, m_ISP_INT.sAnr.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_NBC_ANR, m_ISP_INT.sAnr.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_NBC_ANR, m_ISP_INT.sAnr.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            m_ParamIdx_P2.ANR = m_ISP_INT.sAnr.u2LowerIso_LowerZoom;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                    m_ISP_INT.sAnr.u2UpperIso_UpperZoom,
                    m_ISP_INT.sAnr.u2LowerIso_UpperZoom,
                    m_ISP_INT.sAnr.u2UpperIso_LowerZoom,
                    m_ISP_INT.sAnr.u2LowerIso_LowerZoom);

            ISP_NVRAM_ANR_INT_T rInt_Param;
            rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
            rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
            rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
            rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
            rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
            rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
            rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.NBC_ANR[m_ISP_INT.sAnr.u2UpperIso_UpperZoom];
            rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.NBC_ANR[m_ISP_INT.sAnr.u2LowerIso_UpperZoom];
            rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.NBC_ANR[m_ISP_INT.sAnr.u2UpperIso_LowerZoom];
            rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.NBC_ANR[m_ISP_INT.sAnr.u2LowerIso_LowerZoom];

            AAA_TRACE_ALG(SmoothANR);
            SmoothANR(rInt_Param, anr);
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC_ANR, m_ParamIdx_P2.ANR, m_bMappingQueryFlag);
            anr = m_rIspParam.ISPRegs.NBC_ANR[m_ParamIdx_P2.ANR];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_ANR(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_rIspParam.ISPRegs.NBC_ANR[m_ISP_INT.sAnr.u2UpperIso_LowerZoom],
                                                  m_rIspParam.ISPRegs.NBC_ANR[m_ISP_INT.sAnr.u2LowerIso_LowerZoom], anr);
            }
            m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
        }

        // set ANR_LCE_LINK
//#warning "SWNR linker error"
#if 0
        SwNRParam::getInstance(m_i4SensorIdx)->setANR_LCE_LINK(static_cast<MBOOL>(anr.con1.bits.ANR_LCE_LINK));
        m_pLscMgr->setSwNr();
#endif
        // Load it to ISP manager buffer.
        //rNbc.put(anr);

        // Load it to ISP manager buffer.
         rNbc.put(anr);

         ISP_NBC_LTM_SW_PARAM_T sw_anr = m_rIspParam.ISPRegs.NBC_LCE_LINK[0];

         getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_NBC_LCE_LINK, m_ISP_INT.sSwAnrLtm.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
         getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_NBC_LCE_LINK, m_ISP_INT.sSwAnrLtm.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
         getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_NBC_LCE_LINK, m_ISP_INT.sSwAnrLtm.u2UpperIso_LowerZoom, m_bMappingQueryFlag);
         getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC_LCE_LINK, m_ISP_INT.sSwAnrLtm.u2LowerIso_LowerZoom, m_bMappingQueryFlag);

         CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                   __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                    m_ISP_INT.sSwAnrLtm.u2UpperIso_UpperZoom,
                    m_ISP_INT.sSwAnrLtm.u2LowerIso_UpperZoom,
                    m_ISP_INT.sSwAnrLtm.u2UpperIso_LowerZoom,
                    m_ISP_INT.sSwAnrLtm.u2LowerIso_LowerZoom);


         if((m_ISP_INT.sSwAnrLtm.u2UpperIso_LowerZoom < NVRAM_LCE_LINK_NUM) &&
            (m_ISP_INT.sSwAnrLtm.u2LowerIso_LowerZoom < NVRAM_LCE_LINK_NUM))
         {
             AAA_TRACE_ALG(SmoothNBC_LTM_SW);
             SmoothNBC_LTM_SW(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                       m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                       m_rIspParam.ISPRegs.NBC_LCE_LINK[m_ISP_INT.sSwAnrLtm.u2UpperIso_LowerZoom], //  settings for upper ISO
                       m_rIspParam.ISPRegs.NBC_LCE_LINK[m_ISP_INT.sSwAnrLtm.u2LowerIso_LowerZoom], //  settings for lower ISO
                       sw_anr);  // Output
             AAA_TRACE_END_ALG;
         }

         rNbc.putSwParam(sw_anr);

    }


    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR_TBL(const RAWIspCamInfo& rIspCamInfo)
{

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.anr_tbl.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_MGR_NBC_T& rNbc = ISP_MGR_NBC_T::getInstance(m_eSensorDev);

    if(bDisable || (!rNbc.isANR1Enable()) ||
       (rIspCamInfo.bBypassNR)){
        rNbc.setANRTBLEnable(MFALSE);
    }
    else{
        rNbc.setANRTBLEnable(MTRUE);

        ISP_NVRAM_ANR_LUT_T anr_tbl;

        getISPIndex(eIDX_LowerLV, rIspCamInfo, EModule_FD_ANR, m_ISP_INT.sFD_ANR.u2LowerLv_UpperCt, m_bMappingQueryFlag);
		getISPIndex(eIDX_UpperLV, rIspCamInfo, EModule_FD_ANR, m_ISP_INT.sFD_ANR.u2UpperLv_LowerCt, m_bMappingQueryFlag);
		getISPIndex(eIDX_UpperLV, rIspCamInfo, EModule_NBC_TBL, m_ISP_INT.NBC_TBL.u2UpperLv_LowerCt, m_bMappingQueryFlag);
        getISPIndex(eIDX_LowerLV, rIspCamInfo, EModule_NBC_TBL, m_ISP_INT.NBC_TBL.u2LowerLv_UpperCt, m_bMappingQueryFlag);

        CAM_LOGD_IF(m_bDebugEnable,"%s(): i4RealLightValue_x10 = %d, i4CCT = %d, FDANR_LowerLv = %d, FdANR_UpperLv = %d, ANR_TBL_LowerLv = %d, ANR_TBL_UpperLv = %d\n",
           __FUNCTION__, rIspCamInfo.rAEInfo.i4RealLightValue_x10, rIspCamInfo.rAWBInfo.i4CCT,
           m_ISP_INT.sFD_ANR.u2LowerLv_UpperCt,
           m_ISP_INT.sFD_ANR.u2UpperLv_LowerCt,
           m_ISP_INT.NBC_TBL.u2LowerLv_UpperCt,
           m_ISP_INT.NBC_TBL.u2UpperLv_LowerCt);

        SmoothANR_TBL(rIspCamInfo.rAEInfo.i4RealLightValue_x10,
            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L],
            m_rIspParam.ISPRegs.FD_ANR[m_ISP_INT.sFD_ANR.u2UpperLv_LowerCt],
            m_rIspParam.ISPRegs.FD_ANR[m_ISP_INT.sFD_ANR.u2UpperLv_LowerCt],
            m_fd_tuning_info,
            m_rIspParam.ISPRegs.NBC_TBL[m_ISP_INT.NBC_TBL.u2UpperLv_LowerCt],
            m_rIspParam.ISPRegs.NBC_TBL[m_ISP_INT.NBC_TBL.u2LowerLv_UpperCt],
            anr_tbl);  // Output

        rNbc.putANR_TBL_Backup(anr_tbl);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NDG2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ndg2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_NDG2_T& rNdg2 = ISP_MGR_NDG2_T::getInstance(m_eSensorDev);

    if ((!rNdg2.isCCTEnable()) || (bDisable) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_NDG2_EN)))
    {
        rNdg2.setEnable(MFALSE);
    }
    else{
        rNdg2.setEnable(MTRUE);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR2(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.anr2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||
        (!rNbc2.isCCTANR2Enable()) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL)       ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_ANR2_EN))||
        (rIspCamInfo.bBypassNR))
    {

        rNbc2.setANR2Enable(MFALSE);
    }

    else {
        rNbc2.setANR2Enable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC2_ANR2, m_ISP_INT.sAnr2.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_ANR2_T anr2 = m_rIspParam.ISPRegs.NBC2_ANR2[m_ISP_INT.sAnr2.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_NBC2_ANR2, m_ISP_INT.sAnr2.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_NBC2_ANR2, m_ISP_INT.sAnr2.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_NBC2_ANR2, m_ISP_INT.sAnr2.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            m_ParamIdx_P2.ANR2 = m_ISP_INT.sAnr2.u2LowerIso_LowerZoom;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                    m_ISP_INT.sAnr2.u2UpperIso_UpperZoom,
                    m_ISP_INT.sAnr2.u2LowerIso_UpperZoom,
                    m_ISP_INT.sAnr2.u2UpperIso_LowerZoom,
                    m_ISP_INT.sAnr2.u2LowerIso_LowerZoom);

            ISP_NVRAM_ANR2_INT_T rInt_Param;
            rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
            rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
            rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
            rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
            rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
            rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
            rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.NBC2_ANR2[m_ISP_INT.sAnr2.u2UpperIso_UpperZoom];
            rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.NBC2_ANR2[m_ISP_INT.sAnr2.u2LowerIso_UpperZoom];
            rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.NBC2_ANR2[m_ISP_INT.sAnr2.u2UpperIso_LowerZoom];
            rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.NBC2_ANR2[m_ISP_INT.sAnr2.u2LowerIso_LowerZoom];

            AAA_TRACE_ALG(SmoothANR2);
            SmoothANR2(rInt_Param, anr2);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC2_ANR2, m_ParamIdx_P2.ANR2, m_bMappingQueryFlag);
            anr2 = m_rIspParam.ISPRegs.NBC2_ANR2[m_ParamIdx_P2.ANR2];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_ANR2(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_rIspParam.ISPRegs.NBC2_ANR2[m_ISP_INT.sAnr2.u2UpperIso_LowerZoom],
                                                  m_rIspParam.ISPRegs.NBC2_ANR2[m_ISP_INT.sAnr2.u2LowerIso_LowerZoom], anr2);
            }
            m_pIspTuningCustom->refine_ANR2(rIspCamInfo, m_IspNvramMgr, anr2);
        }

        // Load it to ISP manager buffer.
        rNbc2.put(anr2);
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||
        (!rNbc2.isCCTCCREnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_CCR_EN))) {
        rNbc2.setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rNbc2.setCCREnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC2_CCR, m_ISP_INT.sCcr.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_CCR_T ccr = m_rIspParam.ISPRegs.NBC2_CCR[m_ISP_INT.sCcr.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_NBC2_CCR, m_ISP_INT.sCcr.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_NBC2_CCR, m_ISP_INT.sCcr.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_NBC2_CCR, m_ISP_INT.sCcr.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                    m_ISP_INT.sCcr.u2UpperIso_UpperZoom,
                    m_ISP_INT.sCcr.u2LowerIso_UpperZoom,
                    m_ISP_INT.sCcr.u2UpperIso_LowerZoom,
                    m_ISP_INT.sCcr.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothCCR);
            SmoothCCR(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                    m_rIspParam.ISPRegs.NBC2_CCR[m_ISP_INT.sCcr.u2UpperIso_LowerZoom], // CCR settings for upper ISO
                    m_rIspParam.ISPRegs.NBC2_CCR[m_ISP_INT.sCcr.u2LowerIso_LowerZoom], // CCR settings for lower ISO
                    ccr);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC2_CCR, m_ParamIdx_P2.CCR, m_bMappingQueryFlag);
            ccr = m_rIspParam.ISPRegs.NBC2_CCR[m_ParamIdx_P2.CCR];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCR(rIspCamInfo, m_IspNvramMgr, ccr);
        }

        // Load it to ISP manager buffer.
        rNbc2.put(ccr);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_BOK(const RAWIspCamInfo& rIspCamInfo)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.bok.disable", value, "1"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||(!rNbc2.isCCTBOKEnable()) ||
        (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_YUV_Reprocess))
    {
        rNbc2.setBOKEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rNbc2.setBOKEnable(MTRUE);


        // Get default NVRAM parameter
        ISP_NVRAM_BOK_T bok = m_IspNvramMgr.getBOK(m_ParamIdx_P2.BOK);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BOK(rIspCamInfo, m_IspNvramMgr, bok);
        }

        // Load it to ISP manager buffer.
        rNbc2.put(bok);
    }
#endif
    return  MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_ABF(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.abf.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||(!rNbc2.isCCTABFEnable()) ||
        (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_YUV_Reprocess) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_ABF_EN))||
        (rIspCamInfo.bBypassNR))
    {
        rNbc2.setABFEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rNbc2.setABFEnable(MTRUE);


        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC2_ABF, m_ISP_INT.sAbf.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_ABF_T abf = m_rIspParam.ISPRegs.NBC2_ABF[m_ISP_INT.sAbf.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if(isIspInterpolation(rIspCamInfo))
        {
            //getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_NBC2_ABF, m_ISP_INT.sAbf.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            //getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_NBC2_ABF, m_ISP_INT.sAbf.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_NBC2_ABF, m_ISP_INT.sAbf.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                      __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                       m_ISP_INT.sAbf.u2UpperIso_UpperZoom,
                       m_ISP_INT.sAbf.u2LowerIso_UpperZoom,
                       m_ISP_INT.sAbf.u2UpperIso_LowerZoom,
                       m_ISP_INT.sAbf.u2LowerIso_LowerZoom);

            AAA_TRACE_ALG(SmoothABF);

            SmoothABF(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                      m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                      m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                      m_rIspParam.ISPRegs.NBC2_ABF[m_ISP_INT.sAbf.u2UpperIso_LowerZoom], // DBS settings for upper ISO
                      m_rIspParam.ISPRegs.NBC2_ABF[m_ISP_INT.sAbf.u2LowerIso_LowerZoom], // DBS settings for lower ISO
                      abf);  // Output

            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NBC2_ABF, m_ParamIdx_P2.ABF, m_bMappingQueryFlag);
            abf = m_rIspParam.ISPRegs.NBC2_ABF[m_ParamIdx_P2.ABF];
        }
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ABF(rIspCamInfo, m_IspNvramMgr, abf);
        }

        // Load it to ISP manager buffer.
        rNbc2.put(abf);
    }
    return  MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_PCA(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
#if MTK_CAM_NEW_NVRAM_SUPPORT
    property_get("vendor.isp.pca.disable", value, "1"); // 0: enable, 1: disable
#else
    property_get("vendor.isp.pca.disable", value, "0"); // 0: enable, 1: disable
#endif
    MBOOL bDisable = atoi(value);
    ISP_MGR_PCA_T& rPca = ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode);

//    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);
/*
    if (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_VFB_PostProc) {
        m_ePCAMode = EPCAMode_360BIN;
    }
    else {
        m_ePCAMode = EPCAMode_180BIN;
    }
*/
    m_ePCAMode = EPCAMode_180BIN;

    if ((bDisable) || (!rPca.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_PCA_EN)))
    {
        rPca.setEnable(MFALSE);

        return  MTRUE;
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rPca.setEnable(MTRUE);


#if MTK_CAM_NEW_NVRAM_SUPPORT
#else
        //dynamic color
        ISP_NVRAM_PCA_LUT_T pca = m_rIspParam.ISPColorTbl.ISPColor.PCA_LUT[0][0];

        MBOOL bIsCapture = m_pIspTuningCustom->is_to_invoke_capture_in_color(rIspCamInfo);
        if (m_rIspParam.ISPColorTbl.ISPColor.COLOR_Method && m_IspInterpCtrl)
        {// Smooth PCA
#if 0
            SmoothPCA_TBL(  bIsCapture,
                            m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[rIspCamInfo.eIdx_CT_U],
                            m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[rIspCamInfo.eIdx_CT_L],
                            rIspCamInfo.eIdx_CT_U, rIspCamInfo.eIdx_CT_L,
                            m_rIspParam.ISPColorTbl.ISPColor.PCA_LUT,
                            m_rIspParam.ISPColorTbl.ISPColor.rPcaParam,
                            rIspCamInfo,
                            pca, m_ISP_ALG_VER.sPCA);
#endif
        }
        // (2) Invoke callback for customers to modify.
        if  (isDynamicTuning())
        {   // Dynamic Tuning: Enable
                   //m_pPcaMgr->setIdx(static_cast<MUINT32>(m_ISP_DYM.sPca_tbl_Idx));
        }
        m_pPcaMgr->loadConfig();
        rPca.loadLut(reinterpret_cast<MUINT32*>(&pca.lut[0]));
#endif
    }

    if (m_ePCAMode == EPCAMode_360BIN) { // for VFB
//#warning "VFB linker error"
#if 0
        CAM_LOGD_IF(m_bDebugEnable,"%s: loading vFB PCA (0x%x) ...", __FUNCTION__, halVFBTuning::getInstance().mHalVFBTuningGetPCA());
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(halVFBTuning::getInstance().mHalVFBTuningGetPCA());
#endif
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_HFG(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.hfg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_HFG_T& rHfg = ISP_MGR_HFG_T::getInstance(m_eSensorDev);

    if ((bDisable) ||(!rHfg.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_HFG_EN)))
    {
        rHfg.setEnable(MFALSE);
    }
    else{
        rHfg.setEnable(MTRUE);
        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_HFG, m_ISP_INT.sHfg.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_HFG_T hfg = m_rIspParam.ISPRegs.HFG[m_ISP_INT.sHfg.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_HFG, m_ISP_INT.sHfg.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_HFG, m_ISP_INT.sHfg.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_HFG, m_ISP_INT.sHfg.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                    m_ISP_INT.sHfg.u2UpperIso_UpperZoom,
                    m_ISP_INT.sHfg.u2LowerIso_UpperZoom,
                    m_ISP_INT.sHfg.u2UpperIso_LowerZoom,
                    m_ISP_INT.sHfg.u2LowerIso_LowerZoom);

            ISP_NVRAM_HFG_INT_T rInt_Param;
            rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
            rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
            rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
            rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
            rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
            rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
            rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.HFG[m_ISP_INT.sHfg.u2UpperIso_UpperZoom];
            rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.HFG[m_ISP_INT.sHfg.u2LowerIso_UpperZoom];
            rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.HFG[m_ISP_INT.sHfg.u2UpperIso_LowerZoom];
            rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.HFG[m_ISP_INT.sHfg.u2LowerIso_LowerZoom];

            AAA_TRACE_ALG(SmoothHFG);
            SmoothHFG(rInt_Param, hfg);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_HFG, m_ParamIdx_P2.HFG, m_bMappingQueryFlag);
            hfg = m_rIspParam.ISPRegs.HFG[m_ParamIdx_P2.HFG];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_HFG(rIspCamInfo, m_IspNvramMgr, hfg);
        }
        // Load it to ISP manager buffer.
        rHfg.put(hfg);
    }

    return  MTRUE;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EE(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ee.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_SEEE_T& rSeee = ISP_MGR_SEEE_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    CAM_LOGD_IF(m_bDebugEnable,"EE level %d mode %d", rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, rIspCamInfo.eEdgeMode);

    //EE
    if ((bDisable) || (!rSeee.isCCTEEEnable()) ||
        (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                   ||
        ((rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG)&&(!fgRPGEnable))||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_EE_EN)) ||
        (rIspCamInfo.bBypassNR))
    {
        rSeee.setEEEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rSeee.setEEEnable(MTRUE);

        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_EE, m_ISP_INT.sEe.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_EE_T ee = m_rIspParam.ISPRegs.EE[m_ISP_INT.sEe.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if(isIspInterpolation(rIspCamInfo))
        {
            getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_EE, m_ISP_INT.sEe.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_EE, m_ISP_INT.sEe.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_EE, m_ISP_INT.sEe.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

            m_ParamIdx_P2.EE = m_ISP_INT.sEe.u2LowerIso_LowerZoom;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d, rIspCamInfo.eIdx_Zoom_L(%d)\n",
                    __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                    m_ISP_INT.sEe.u2UpperIso_UpperZoom,
                    m_ISP_INT.sEe.u2LowerIso_UpperZoom,
                    m_ISP_INT.sEe.u2UpperIso_LowerZoom,
                    m_ISP_INT.sEe.u2LowerIso_LowerZoom,
                    rIspCamInfo.eIdx_Zoom_L);

            ISP_NVRAM_EE_INT_T rInt_Param;
            rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
            rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
            rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
            rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
            rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
            rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
            rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.EE[m_ISP_INT.sEe.u2UpperIso_UpperZoom];
            rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.EE[m_ISP_INT.sEe.u2LowerIso_UpperZoom];
            rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.EE[m_ISP_INT.sEe.u2UpperIso_LowerZoom];
            rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.EE[m_ISP_INT.sEe.u2LowerIso_LowerZoom];

            AAA_TRACE_ALG(SmoothEE);
            SmoothEE(rInt_Param, ee);  // Output
            AAA_TRACE_END_ALG;
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_EE, m_ParamIdx_P2.EE, m_bMappingQueryFlag);
            ee = m_rIspParam.ISPRegs.EE[m_ParamIdx_P2.EE];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_EE(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_rIspParam.ISPRegs.EE[m_ISP_INT.sEe.u2UpperIso_LowerZoom],
                                                  m_rIspParam.ISPRegs.EE[m_ISP_INT.sEe.u2LowerIso_LowerZoom],
                                                  ee);
            }
            m_pIspTuningCustom->refine_EE(rIspCamInfo, m_IspNvramMgr, ee);

            /*if (rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
            {
                // User setting
                m_pIspTuningCustom->userSetting_EE(rIspCamInfo, rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);
            }*/
        }

        // Load it to ISP manager buffer.
        rSeee.put(ee);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EFFECT(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.g2c.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_G2C_T& rG2c = ISP_MGR_G2C_T::getInstance(m_eSensorDev);
    ISP_MGR_SEEE_T& rSeee = ISP_MGR_SEEE_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] effect(%d)", __FUNCTION__, rIspCamInfo.eIdx_Effect);

    char value2[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.se.disable", value2, "0"); // 0: enable, 1: disable
    MBOOL bDisable2 = atoi(value2);

    //SE
    if (bDisable2 ||
        (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                   ||
        ((rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG)&&(!fgRPGEnable)) ||
        (rIspCamInfo.eIdx_Effect == MTK_CONTROL_EFFECT_MODE_OFF) ||
        (rIspCamInfo.i4P2InImgFmt)||
        (rIspCamInfo.bBypassNR))
    {
        rSeee.setSEEnable(MFALSE);
    }
    else
    {
        rSeee.setSEEnable(MTRUE);
    }

    //G2C
    if ((bDisable) || (!rG2c.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_G2C_EN))
        //||(m_u4RawFmtType == SENSOR_RAW_MONO)
       )
    {
        rG2c.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta) {
        rG2c.setEnable(MTRUE);
        //}

        ISP_NVRAM_G2C_T g2c;
        ISP_NVRAM_G2C_SHADE_T g2c_shade;
        ISP_NVRAM_SE_T se;
        //no use
        ISP_NVRAM_GGM_T ggm;

        // Get ISP HW buffer
        getIspHWBuf(m_eSensorDev, g2c);
        getIspHWBuf(m_eSensorDev, g2c_shade);
        getIspHWBuf(m_eSensorDev, se);

        CAM_LOGD_IF(m_bDebugEnable, "[%s] eIdx_Hue(%d) eIdx_Sat(%d) eIdx_Bright(%d) eIdx_Contrast(%d)"
            , __FUNCTION__
            , rIspCamInfo.rIspUsrSelectLevel.eIdx_Hue
            , rIspCamInfo.rIspUsrSelectLevel.eIdx_Sat
            , rIspCamInfo.rIspUsrSelectLevel.eIdx_Bright
            , rIspCamInfo.rIspUsrSelectLevel.eIdx_Contrast);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->userSetting_EFFECT(rIspCamInfo, rIspCamInfo.eIdx_Effect, rIspCamInfo.rIspUsrSelectLevel, g2c, g2c_shade, se, ggm);
        }
        // Load it to ISP manager buffer.
        rG2c.put(g2c);
        ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).put(g2c_shade);
        ISP_MGR_SEEE_T::getInstance(m_eSensorDev).put(se);
    }

    return  MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D(const RAWIspCamInfo& rIspCamInfo)
{
    ISP_MGR_NR3D_T& rTnr = ISP_MGR_NR3D_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NR3D, m_ISP_INT.sNr3d.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
    ISP_NVRAM_NR3D_T nr3d = m_rIspParam.ISPRegs.NR3D[m_ISP_INT.sNr3d.u2LowerIso_LowerZoom];

    ISP_NR3D_SMOOTH_INFO_T NR3D_SW_Params;

    NR3D_SW_Params.Device_ID                = m_eSensorDev;
    NR3D_SW_Params.M_Info                   = rIspCamInfo.NR3D_Data;
    NR3D_SW_Params.C_Paras.GMV_level_th_def = 200;
    NR3D_SW_Params.C_Paras.GMV_level_th     = 200;
    NR3D_SW_Params.C_Paras.GMV_sm_th        = 112;
    NR3D_SW_Params.C_Paras.GMV_sm_th_up     = 8;
    NR3D_SW_Params.C_Paras.GMV_sm_th_lo     = 10;
    NR3D_SW_Params.C_Paras.GMV_lv_th_up     = 180;
    NR3D_SW_Params.C_Paras.GMV_lv_th_lo     = 50;
    NR3D_SW_Params.C_Paras.GMV_min_str      = 102;
    NR3D_SW_Params.C_Paras.ISO_upper        = 800;
    NR3D_SW_Params.C_Paras.Conf_th          = 10;

    if (isIspInterpolation(rIspCamInfo))
    {
        getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_NR3D, m_ISP_INT.sNr3d.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
        getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_NR3D, m_ISP_INT.sNr3d.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
        getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_NR3D, m_ISP_INT.sNr3d.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

        CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                m_ISP_INT.sNr3d.u2UpperIso_UpperZoom,
                m_ISP_INT.sNr3d.u2LowerIso_UpperZoom,
                m_ISP_INT.sNr3d.u2UpperIso_LowerZoom,
                m_ISP_INT.sNr3d.u2LowerIso_LowerZoom);

        ISP_NVRAM_NR3D_INT_T rInt_Param;
        rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
        rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
        rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
        rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
        rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
        rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
        rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.NR3D[m_ISP_INT.sNr3d.u2UpperIso_UpperZoom];
        rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.NR3D[m_ISP_INT.sNr3d.u2LowerIso_UpperZoom];
        rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.NR3D[m_ISP_INT.sNr3d.u2UpperIso_LowerZoom];
        rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.NR3D[m_ISP_INT.sNr3d.u2LowerIso_LowerZoom];

        SmoothNR3D(rInt_Param, nr3d, NR3D_SW_Params);  // Output
    }
    else{
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_NR3D, m_ParamIdx_P2.NR3D, m_bMappingQueryFlag);
        nr3d = m_rIspParam.ISPRegs.NR3D[m_ParamIdx_P2.NR3D];
    }


    MINT32 GMVLevelTh = rTnr.getGMVLevelTh();
    m_pIspTuningCustom->adaptive_NR3D_setting(rIspCamInfo, NR3D_SW_Params, nr3d, GMVLevelTh, m_eSensorDev);
    rTnr.setGMVLevelTh(GMVLevelTh);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR3D(rIspCamInfo, m_IspNvramMgr, nr3d);
    }

    // Load it to ISP manager buffer
    rTnr.put(nr3d);
    return  MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_COLOR(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.color.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_NR3D_T& rNr3d = ISP_MGR_NR3D_T::getInstance(m_eSensorDev);
    CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.eColorCorrectionMode = %d",__FUNCTION__,rIspCamInfo.eColorCorrectionMode);

    if ((bDisable) || (!rNr3d.isCCTColorEnable()) || (rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_COLOR_EN)))
    {
        rNr3d.setColorEnable(MFALSE);
    }
    else{
        rNr3d.setColorEnable(MTRUE);
        //dynamic color
        ISP_NVRAM_COLOR_T color = m_rIspParam.ISPColorTbl.COLOR[0].COLOR_TBL;

        if( m_rIspParam.ISPColorTbl.COMM.COLOR.COLOR_Method && m_IspInterpCtrl)
        {
            getISPIndex(eIDX_LowerLv_LowerCt, rIspCamInfo, EModule_COLOR, m_ISP_INT.sColor.u2LowerLv_LowerCt, m_bMappingQueryFlag);
            getISPIndex(eIDX_LowerLv_UpperCt, rIspCamInfo, EModule_COLOR, m_ISP_INT.sColor.u2LowerLv_UpperCt, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperLv_LowerCt, rIspCamInfo, EModule_COLOR, m_ISP_INT.sColor.u2UpperLv_LowerCt, m_bMappingQueryFlag);
            getISPIndex(eIDX_UpperLv_UpperCt, rIspCamInfo, EModule_COLOR, m_ISP_INT.sColor.u2UpperLv_UpperCt, m_bMappingQueryFlag);
            getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo, EModule_COLOR_PARAM, m_ISP_INT.sColor.u2ParamIdx, m_bMappingQueryFlag);

            MBOOL bIsCapture = m_pIspTuningCustom->is_to_invoke_capture_in_color(rIspCamInfo);
            ColorInfo rColorInfo;
            memset(&rColorInfo, 0, sizeof(ColorInfo));

            rColorInfo.RealLV = rIspCamInfo.rAEInfo.i4RealLightValue_x10;
            rColorInfo.AWB_NoPrefGain[0] = rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4R;
            rColorInfo.AWB_NoPrefGain[1] = rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4G;
            rColorInfo.AWB_NoPrefGain[2] = rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4B;
            rColorInfo.NoPrefCCT = rIspCamInfo.rAWBInfo.i4CCT;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): i4RealLightValue_x10 = %d, i4CCT = %d, u2LowerLv_LowerCt = %d, u2LowerLv_UpperCt = %d, u2UpperLv_LowerCt = %d, u2UpperLv_UpperCt = %d, u2ParamIdx = %d\n",
                       __FUNCTION__, rIspCamInfo.rAEInfo.i4RealLightValue_x10, rIspCamInfo.rAWBInfo.i4CCT,
                       m_ISP_INT.sColor.u2LowerLv_LowerCt,
                       m_ISP_INT.sColor.u2LowerLv_UpperCt,
                       m_ISP_INT.sColor.u2UpperLv_LowerCt,
                       m_ISP_INT.sColor.u2UpperLv_UpperCt,
                       m_ISP_INT.sColor.u2ParamIdx);

            SmoothCOLOR( bIsCapture,
                         m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_U],
                         m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_L],
                         m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
                         m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L],
                         rColorInfo,
                         m_fd_tuning_info,
                         m_rIspParam.ISPColorTbl.COLOR[m_ISP_INT.sColor.u2LowerLv_LowerCt],
                         m_rIspParam.ISPColorTbl.COLOR[m_ISP_INT.sColor.u2LowerLv_UpperCt],
                         m_rIspParam.ISPColorTbl.COLOR[m_ISP_INT.sColor.u2UpperLv_LowerCt],
                         m_rIspParam.ISPColorTbl.COLOR[m_ISP_INT.sColor.u2UpperLv_UpperCt],
                         m_rIspParam.ISPColorTbl.COLOR_PARAM[m_ISP_INT.sColor.u2ParamIdx],
                         color,
                         m_ISP_ALG_VER.sCOLOR,
                         m_bSmoothColor_FirstTimeBoot,
                         m_SmoothColor_Prv);
        }
        rNr3d.put(color);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MFB(const RAWIspCamInfo& rIspCamInfo)
{
    // Get default NVRAM parameter
    getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_MFB, m_ISP_INT.sMfb.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
    ISP_NVRAM_MFB_T mfb = m_rIspParam.ISPRegs.MFB[m_ISP_INT.sMfb.u2LowerIso_LowerZoom];

    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    //Interpolation ISP module
    if (isIspInterpolation(rIspCamInfo))
    {
         getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_MFB, m_ISP_INT.sMfb.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
         getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_MFB, m_ISP_INT.sMfb.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
         getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_MFB, m_ISP_INT.sMfb.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

         ISP_NVRAM_MFB_INT_T rInt_Param;
         rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
         rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
         rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
         rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
         rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
         rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
         rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.MFB[m_ISP_INT.sMfb.u2UpperIso_UpperZoom];
         rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.MFB[m_ISP_INT.sMfb.u2LowerIso_UpperZoom];
         rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.MFB[m_ISP_INT.sMfb.u2UpperIso_LowerZoom];
         rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.MFB[m_ISP_INT.sMfb.u2LowerIso_LowerZoom];

         SmoothMFB(rInt_Param, mfb);  // Output
    }
    else{
         getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_MFB, m_ParamIdx_P2.MFB, m_bMappingQueryFlag);
         mfb = m_rIspParam.ISPRegs.MFB[m_ParamIdx_P2.MFB];
    }

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MFB(rIspCamInfo, m_IspNvramMgr, mfb);
    }

    // Load it to ISP manager buffer
    ISP_MGR_MFB_T::getInstance(m_eSensorDev).put(mfb);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Mixer3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MIXER3(const RAWIspCamInfo& rIspCamInfo)
{

    if (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_MIXER3_EN)
    {
        // Get default NVRAM parameter
        getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_MIXER3, m_ISP_INT.sMix3.u2LowerIso_LowerZoom, m_bMappingQueryFlag);
        ISP_NVRAM_MIXER3_T mixer3 = m_rIspParam.ISPRegs.MIXER3[m_ISP_INT.sMix3.u2LowerIso_LowerZoom];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
             getISPIndex(eIDX_UpperIso_UpperZoom , rIspCamInfo, EModule_MIXER3, m_ISP_INT.sMix3.u2UpperIso_UpperZoom, m_bMappingQueryFlag);
             getISPIndex(eIDX_LowerIso_UpperZoom , rIspCamInfo, EModule_MIXER3, m_ISP_INT.sMix3.u2LowerIso_UpperZoom, m_bMappingQueryFlag);
             getISPIndex(eIDX_UpperIso_LowerZoom , rIspCamInfo, EModule_MIXER3, m_ISP_INT.sMix3.u2UpperIso_LowerZoom, m_bMappingQueryFlag);

             CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                        __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                        m_ISP_INT.sMix3.u2UpperIso_UpperZoom,
                        m_ISP_INT.sMix3.u2LowerIso_UpperZoom,
                        m_ISP_INT.sMix3.u2UpperIso_LowerZoom,
                        m_ISP_INT.sMix3.u2LowerIso_LowerZoom);

            ISP_NVRAM_MIXER3_INT_T rInt_Param;
            rInt_Param.u4RealISO           = rIspCamInfo.rAEInfo.u4RealISOValue;
            rInt_Param.u4UpperISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
            rInt_Param.u4LowerISO          = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
            rInt_Param.i4ZoomRatio_x100    = rIspCamInfo.i4ZoomRatio_x100;
            rInt_Param.u4UpperZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
            rInt_Param.u4LowerZoom         = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
            rInt_Param.rUpperIso_UpperZoom = m_rIspParam.ISPRegs.MIXER3[m_ISP_INT.sMix3.u2UpperIso_UpperZoom];
            rInt_Param.rLowerIso_UpperZoom = m_rIspParam.ISPRegs.MIXER3[m_ISP_INT.sMix3.u2LowerIso_UpperZoom];
            rInt_Param.rUpperIso_LowerZoom = m_rIspParam.ISPRegs.MIXER3[m_ISP_INT.sMix3.u2UpperIso_LowerZoom];
            rInt_Param.rLowerIso_LowerZoom = m_rIspParam.ISPRegs.MIXER3[m_ISP_INT.sMix3.u2LowerIso_LowerZoom];

            SmoothMIXER3(rInt_Param, mixer3);  // Output
        }
        else{
            getISPIndex(eIDX_LowerIso_LowerZoom , rIspCamInfo, EModule_MIXER3, m_ParamIdx_P2.MIXER3, m_bMappingQueryFlag);
            mixer3 = m_rIspParam.ISPRegs.MIXER3[m_ParamIdx_P2.MIXER3];
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_MIXER3(rIspCamInfo, m_IspNvramMgr, mixer3);
        }

        // Load it to ISP manager buffer
        ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).put(mixer3);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LCE(const RAWIspCamInfo& rIspCamInfo)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.lce.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    MBOOL bBufUnSafe = ((m_pLCSBuffer == NULL) || (m_pLCSBuffer->getImageBufferHeap()==NULL));

    ISP_MGR_LCE_T& rLce = ISP_MGR_LCE_T::getInstance(m_eSensorDev);

    if ((bDisable) ||(!rLce.isCCTEnable()) ||  bBufUnSafe ||
        (!(m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_LCE_EN)) ||
        (rIspCamInfo.bBypassLCE) ||
        (rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) ||
        (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE))
    {
        rLce.setEnable(MFALSE);

        if(bBufUnSafe)
            CAM_LOGW("[%s] get LCE image buffer fail !", __func__);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            rLce.setEnable(MTRUE);

        ISP_NVRAM_LCE_T lce;
        ISP_NVRAM_PGN_T pgn;
        ISP_MGR_PGN::getInstance(m_eSensorDev).get(pgn);
        ISP_NVRAM_CCM_T ccm;
        ISP_MGR_CCM::getInstance(m_eSensorDev).get(ccm);

        rLce.setLCEISize(rIspCamInfo.rLCS_Info.rOutSetting);

        if(rIspCamInfo.u1P2TuningUpdate == 5){

            RAWIspCamInfo rIspCamInfo_ = rIspCamInfo;
            rIspCamInfo_.rMapping_Info.eIspProfile = EIspProfile_Preview;
            getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo_, EModule_LCE, m_ISP_INT.u2LCE, m_bMappingQueryFlag);
            getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo_, EModule_GMA, m_ISP_INT.u2GGM, m_bMappingQueryFlag);
        }
        else{
            // Get Value from algo
            getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo, EModule_LCE, m_ISP_INT.u2LCE, m_bMappingQueryFlag);
            getISPIndex(eIDX_ISP_Scenario_Num, rIspCamInfo, EModule_GMA, m_ISP_INT.u2GGM, m_bMappingQueryFlag);
        }
        AAA_TRACE_ALG(calculateLCE);

        if(rIspCamInfo.u1P2TuningUpdate != 2){
            m_pLceMgr->calculateLCE(rIspCamInfo, m_ISP_INT.u2LCE, m_ISP_INT.u2GGM,
                                    &lce, &m_LceExifInfo,
                                    m_bSlave, m_ISP_Sync_Info,
                                    rIspCamInfo.rLCS_Info.rOutSetting,
                                    rIspCamInfo.rLCS_Info.rInSetting.lcs, pgn, ccm, (MUINT16*)m_pLCSBuffer->getImageBufferHeap()->getBufVA(0));
        }
        AAA_TRACE_END_ALG;

        MSize tempLCEIsize(rIspCamInfo.rLCS_Info.rOutSetting.u4OutWidth, rIspCamInfo.rLCS_Info.rOutSetting.u4OutHeight);

        m_pLCSBuffer->setExtParam(tempLCEIsize, 0);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_LCE(rIspCamInfo, m_IspNvramMgr, lce);
        }
        // Load it to ISP manager buffer.

        rLce.put(lce);
    }

    CAM_LOGD_IF(m_bDebugEnable,"LCE, Final(%d), Profile(%d), Size(%d, %d), Bypass(%d)",
          rLce.isEnable(),
          rIspCamInfo.rMapping_Info.eIspProfile,
          rIspCamInfo.rLCS_Info.rOutSetting.u4OutWidth, rIspCamInfo.rLCS_Info.rOutSetting.u4OutHeight,
          rIspCamInfo.bBypassLCE);


    MUINT8 i4LVIdx_H;
    MUINT8 i4DRIdx_H;
    m_pLceMgr->getLCE_Gain_EXIF(i4LVIdx_H, i4DRIdx_H);

    MUINT8* tempLCEGain_ParamCover;
    MUINT8 RatioL;

    tempLCEGain_ParamCover = static_cast<MUINT8*>(m_pIspTuningCustom->get_RawHDR_LCEGain_ConverParam());

    if(i4LVIdx_H <= tempLCEGain_ParamCover[0]){
        RatioL = tempLCEGain_ParamCover[1];
    }
    else if(i4LVIdx_H >= tempLCEGain_ParamCover[2]){
        RatioL = tempLCEGain_ParamCover[3];
    }
    else{
        MUINT8 base = tempLCEGain_ParamCover[2] - tempLCEGain_ParamCover[0];
        MUINT8 dominator = i4LVIdx_H - tempLCEGain_ParamCover[0];
        MUINT8 height = tempLCEGain_ParamCover[3] - tempLCEGain_ParamCover[1];
        RatioL = tempLCEGain_ParamCover[1] + (float)height* (float)dominator / (float)base;
    }

    MUINT8 RationFinal;
    if(i4DRIdx_H <= tempLCEGain_ParamCover[4]){
        RationFinal = RatioL;
    }
    else if(i4DRIdx_H >= tempLCEGain_ParamCover[5]){
        RationFinal = tempLCEGain_ParamCover[6];
    }
    else{
        MUINT8 base2 = tempLCEGain_ParamCover[5] - tempLCEGain_ParamCover[4];
        MUINT8 dominator2 = i4DRIdx_H - tempLCEGain_ParamCover[4];
        MUINT8 height2 = tempLCEGain_ParamCover[6] - RatioL;
        RationFinal = RatioL + (float)height2* (float)dominator2 / (float)base2;
    }

    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4LVIdx_H = %d, i4DRIdx_H = %d, RatioL = %d, RationFinal = %d, [0] = %d, [1] = %d, [2] = %d, [3] = %d, [4] = %d, [5] = %d, [6] = %d",
               __FUNCTION__,
               i4LVIdx_H,
               i4DRIdx_H,
               RatioL,
               RationFinal,
               tempLCEGain_ParamCover[0],
               tempLCEGain_ParamCover[1],
               tempLCEGain_ParamCover[2],
               tempLCEGain_ParamCover[3],
               tempLCEGain_ParamCover[4],
               tempLCEGain_ParamCover[5],
               tempLCEGain_ParamCover[6]);

    rLce.setLCEGainConversionRatio(RationFinal);

    return  MTRUE;
}

