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
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_tuning_mgr.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "isp_mgr_af_stat.h"
#include <pca_mgr.h>
#include <ispfeature.h>
#include <isp_interpolation.h>
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
//#include <hwutils/CameraProfile.h>
//#include "vfb_hal_base.h"
//#include <mtkcam/featureio/capturenr.h>
#include <mtkcam/utils/sys/IFileCache.h>

//define log control
#define EN_3A_FLOW_LOG        1
#define EN_3A_SCHEDULE_LOG    2

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

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

    MINT32 i4Magic = i4FrameID;
    MINT32 i4NumInRequestSet;
    RequestSet_T RequestTuningSet =  RequestSet;
    AAA_TRACE_G(applyToHw_perFrame_All);
    AAA_TRACE_NL(applyToHw_perFrame_All);
    m_pTuning->dequeBuffer(RequestTuningSet.vNumberSet.data());

//if (RequestSet.vNumberSet[0] == 1)
{
    for (MINT32 i4InitNum = 0; i4InitNum < m_i4SubsampleCount; i4InitNum++)
    {
        fgRet = ISP_MGR_AE_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_AWB_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_AF_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4InitNum)
            &&  ISP_MGR_PDO_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4InitNum)
            &&  ISP_MGR_FLK_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, m_bFlkEnable, m_eSensorTG, i4InitNum)
            &&  ISP_MGR_SL2F_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.BinInfo, m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RMM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RMG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_CTL_EN_P1_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            // LCS shoud be after CCM, RPG
            &&  ISP_MGR_LCS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)

            ;
    }
}
    AAA_TRACE_END_NL;
    AAA_TRACE_END_G;

/*else
{
    fgRet = ISP_MGR_AE_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_AWB_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_AF_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4SubsampleIdex)
        &&  ISP_MGR_FLK_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, m_bFlkEnable, i4SubsampleIdex)
        &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_RMM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4SubsampleIdex)
        &&  ISP_MGR_RMG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4SubsampleIdex)
        ;
}*/


    m_pTuning->enqueBuffer();

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
applyToHw_PerFrame_P2(MINT32 /*flowType*/, const ISP_INFO_T& rIspInfo, void* pRegBuf)
{
    MBOOL fgRet = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] + pRegBuf(%p)", __FUNCTION__, pRegBuf);

    const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;
    dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);
    ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));

    fgRet = MTRUE
        &&  ISP_MGR_SL2G_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_DBS2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_OBC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_BNR2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_LSC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_SL2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspInfo.rIspP2CropInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        //RNR should be after OBC2 & PGN
        &&  ISP_MGR_RNR_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_UDM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_FLC_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_LCE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NDG_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NDG2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_HFG_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_RMM2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_RMG2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        ;

    if(m_eOperMode == EOperMpde_Factory){
      pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN = 0;
      pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 0;
    }

  // the protection of HW limitations
    ispHWConstraintSet(pReg);   // the protection of HW limitations

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
        &&  prepareHw_PerFrame_LSC()

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

    MINT32 i4RealISO = static_cast<MINT32>(rIspInfo.rCamInfo.rAEInfo.u4RealISOValue);
    MINT32 i4ISO_L = static_cast<MINT32>(rIspInfo.rCamInfo.eIdx_ISO_L);
    MINT32 i4ZoomRatio_x100 = static_cast<MINT32>(rIspInfo.rCamInfo.i4ZoomRatio_x100);
    MBOOL  b1Update = false;
    MBOOL  bRrzOn = rIspInfo.rCamInfo.fgRPGEnable;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] m_bFirstPrepare(%d), i4ISO_L/m_i4PreISO_L(%d/%d), u4RealISO/m_i4PreRealISO(%d/%d), isoThresh(%d), m_i4PreRealISO*isoThresh/100(%d), i4ZoomRatio_x100/(%d)m_i4PreZoomRatio_x100(%d), eControlMode(%d)",
        __FUNCTION__, m_bFirstPrepare, i4ISO_L, m_i4PreISO_L, i4RealISO, m_i4PreRealISO, m_isoThresh, m_i4PreRealISO*m_isoThresh/100, i4ZoomRatio_x100, m_i4PreZoomRatio_x100, rIspInfo.rCamInfo.eControlMode);
    if(m_bFirstPrepare
        || i4ISO_L != m_i4PreISO_L
        || (abs(i4RealISO - m_i4PreRealISO) > m_i4PreRealISO*m_isoThresh/100)
        || (rIspInfo.rCamInfo.eIspProfile != EIspProfile_Preview && rIspInfo.rCamInfo.eIspProfile != EIspProfile_Video)
        || i4ZoomRatio_x100 != m_i4PreZoomRatio_x100
        || rIspInfo.rCamInfo.eIspProfile != m_i4PrePrevProfile
        || bRrzOn != m_bPreRrzOn
        || rIspInfo.rCamInfo.eControlMode == MTK_CONTROL_MODE_OFF)
    {
        CAM_LOGD_IF(m_bDebugEnable, "[%s] Use new HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
        b1Update = true;
        m_i4PreISO_L = i4ISO_L;
        m_i4PreRealISO = i4RealISO;
        m_bFirstPrepare = MFALSE;
        m_i4PreZoomRatio_x100 = i4ZoomRatio_x100;
        m_i4PrePrevProfile = static_cast<MINT32>(rIspInfo.rCamInfo.eIspProfile);
        m_bPreRrzOn = bRrzOn;
    }
    else
    {
        CAM_LOGD_IF(m_bDebugEnable, "[%s] Use previous HW reg(%d)", __FUNCTION__, rIspInfo.rCamInfo.u4Id);
    }
    m_bUseIsoThreshEnable = !b1Update;
    MBOOL fgRet = MTRUE
        &&  prepareHw_PerFrame_SL2G(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_DBS_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_OBC_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_BPC_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_NR1_2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_PDC_2(rIspInfo.rCamInfo) : MTRUE)
        &&  prepareHw_PerFrame_LSC_2(rIspInfo)
        &&  prepareHw_PerFrame_PGN(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_SL2(rIspInfo)
        &&  (b1Update ? prepareHw_PerFrame_RNR(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_UDM(rIspInfo.rCamInfo) : MTRUE)
        &&  prepareHw_PerFrame_FLC(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_CCM(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_GGM(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_NDG(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_ANR(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_ANR_TBL(rIspInfo.rCamInfo) : MTRUE)
        &&  prepareHw_PerFrame_NDG2(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_ANR2(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_CCR(rIspInfo.rCamInfo) : MTRUE)
        //&&  prepareHw_PerFrame_ABF(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_BOK(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_PCA(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_HFG(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_EE(rIspInfo.rCamInfo) : MTRUE)
        &&  prepareHw_PerFrame_EFFECT(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_NR3D(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_COLOR(rIspInfo.rCamInfo)
        &&  (b1Update ? prepareHw_PerFrame_MFB(rIspInfo.rCamInfo) : MTRUE)
        &&  (b1Update ? prepareHw_PerFrame_MIXER3(rIspInfo.rCamInfo) : MTRUE)
        //HLR2 should be after LSC
        //&&  prepareHw_PerFrame_HLR_2(rIspInfo.rCamInfo)
        //LCE should be after HLR_2
        &&  prepareHw_PerFrame_LCE(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_RMM_2(rIspInfo.rCamInfo)
        &&  prepareHw_PerFrame_RMG_2(rIspInfo.rCamInfo)
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
        pReg->DIP_X_ANR_CON1.Bits.ANR_LCE_LINK = 0;
    }

    //Limit 4
    if (!bNBC2_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2C_EN = 0;
    }
    else if(!bSL2C_EN){
        pReg->DIP_X_ANR2_CON1.Bits.ANR2_LCE_LINK = 0;
        pReg->DIP_X_CCR_CON.Bits.CCR_LCE_LINK = 0;
    }

    //Limit 5
    if (!bSEEE_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2D_EN = 0;
    }
    else if(!bSL2D_EN){
        pReg->DIP_X_SEEE_SLNK_CTRL_2.Bits.SEEE_GLUT_LINK_EN = 0;
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

    // Limit 11: for MFLL mix (eDrvScenario_MFB_Mixing)
    MBOOL bMIX3_EN = pReg->DIP_X_CTL_YUV_EN.Bits.MIX3_EN;
    if (bMIX3_EN) {
        pReg->DIP_X_CTL_RGB_EN.Bits.BNR2_EN = 0;
    }

//HHHH  BOK mode N3D SRZ3
    //Limit 10
    if (bNBC2_EN ){
        pReg->DIP_X_ANR2_CON1.Bits.ANR2_MODE = 0;
    }

#if 0
    //PD sensor
    MBOOL bPDC_EN = pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN;
    // PDAF SW limitation
    if (bPDC_EN){
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 1;
    }
    else{
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 0;
    }
#endif

    if(ISP_MGR_BNR2_T::getInstance(m_eSensorDev).getDMGItable() == NULL){
       pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 0;
    }

    if(pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN == 0){
        pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN = 0;
    }
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

    MINT32 i4Rto = m_i4LscMgrRatio;

    MBOOL fgForceUnlock = (i4IsLockRto == IspTuningMgr::E_SHADING_RATIO_UNLOCK) && m_rIspCamInfo.rFlashInfo.isFlash;
    MBOOL fgLockRto = (m_rIspCamInfo.rAEInfo.bAELock || m_rIspCamInfo.rAEInfo.bAELimiter ||
                        (i4IsLockRto != IspTuningMgr::E_SHADING_RATIO_FREE_RUN) || m_rIspCamInfo.rManualAE) && !fgForceUnlock;
    MBOOL fgAutoRto = (i4Rto == -1);

    // Invoke callback for customers to modify.
    if  (m_fgDynamicShading)
    {
        // Dynamic Tuning: Enable
        new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo);
        m_pLscMgr->setCTIdx(new_cct_idx);

        if (fgAutoRto && !fgLockRto)
        {
            i4Rto = m_pIspTuningCustom->evaluate_Shading_Ratio(m_rIspCamInfo);
            m_pLscMgr->setRatio(i4Rto);
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

    CAM_LOGD_IF(m_bDebugEnable, "[%s] (flash, iso, rto)=(%d, %d, %d), auto(%d), lock(%d,%d,%d), dyn(%d), op(%d), PreCapFlashIsLockRto(%d), fgForceUnlock(%d)",
        __FUNCTION__, m_rIspCamInfo.rFlashInfo.isFlash, m_rIspCamInfo.rAEInfo.u4RealISOValue, i4Rto, fgAutoRto,
        m_rIspCamInfo.rAEInfo.bAELock, m_rIspCamInfo.rAEInfo.bAELimiter, m_rIspCamInfo.rManualAE, m_fgDynamicShading, getOperMode(), i4IsLockRto, fgForceUnlock);

    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    //////////////////////////////////////
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
    MINT32 i4Tsf = propertyGet("vendor.debug.lsc_mgr.manual_tsf", defaultLSCValue);
    MINT32 i4OnOff = propertyGet("vendor.debug.lsc_mgr.enable", defaultLSCValue);

    if (i4Tsf != -1){
        m_pLscMgr->setTsfOnOff(i4Tsf ? MTRUE : MFALSE);
    }

    if (i4OnOff != -1){
        m_pLscMgr->setOnOff(i4OnOff ? MTRUE : MFALSE);
    }

    if (!m_pLscMgr->getTsfOnOff())
        m_pLscMgr->updateLsc();
    else
        m_pLscMgr->waitTsfExecDone();

    ILscMgr::SL2_CFG_T rSl2Cfg = m_pLscMgr->getSl2();
    MUINT32 u4RawWd, u4RawHt;
    m_pLscMgr->getRawSize(m_pLscMgr->getSensorMode(), u4RawWd, u4RawHt);
    m_rIspCamInfo.rCropRzInfo.i4FullW = u4RawWd;
    m_rIspCamInfo.rCropRzInfo.i4FullH = u4RawHt;
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
prepareHw_PerFrame_SL2F()
{
    MBOOL bDisable = propertyGet("vendor.isp.sl2f.disable", 1);
    ISP_MGR_SL2F_T& rSl2f = ISP_MGR_SL2F_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgSL2F = rSl2f.isCCTEnable();

    if ((bDisable) || (!rSl2f.isCCTEnable()) ||
        (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_SL2FG_EN)))
    {
        rSl2f.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rSl2f.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_SL2_T sl2f = m_IspNvramMgr.getSL2F(m_ParamIdx_P1.SL2F);

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
prepareHw_PerFrame_DBS()
{
    MBOOL bDisable = propertyGet("vendor.isp.dbs.disable", 0);
    ISP_MGR_DBS_T& rDbs = ISP_MGR_DBS_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgDBS = rDbs.isCCTEnable();

    if ((bDisable) || (!rDbs.isCCTEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_DBS_EN))|| m_DualPD_PureRaw )
    {
        rDbs.setEnable(MFALSE);
    }
    else {
        if(!(m_rIspCamInfo.fgHDR)){
        //if (getOperMode() != EOperMode_Meta)
        rDbs.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS(m_ParamIdx_P1.DBS);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U || m_rIspCamInfo.eIdx_Zoom_L != m_rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sDbs.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).DBS;
                   m_ISP_INT.sDbs.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).DBS;
                   m_ISP_INT.sDbs.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).DBS;
                   m_ISP_INT.sDbs.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).DBS;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sDbs.u2UpperIso_UpperZoom,
                             m_ISP_INT.sDbs.u2LowerIso_UpperZoom,
                             m_ISP_INT.sDbs.u2UpperIso_LowerZoom,
                             m_ISP_INT.sDbs.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothDBS);
                  AAA_TRACE_NL(SmoothDBS);
                  SmoothDBS(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs.u2UpperIso_LowerZoom), // DBS settings for upper ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs.u2LowerIso_LowerZoom), // DBS settings for lower ISO
                            dbs);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_DBS(m_rIspCamInfo, m_IspNvramMgr, dbs);
        }

        // Load it to ISP manager buffer.
        rDbs.put(dbs);
    }
        else{
            MUINT32 DBS_Iso = m_pIspTuningCustom->get_HdrDbsNewISO(m_rIspCamInfo);

            EIndex_ISO_T temp_IDX = map_ISO_value2index(DBS_Iso);

            RAWIspCamInfo temp_CamInfo;

            get_ISOIdx_neighbor(temp_CamInfo, temp_IDX, DBS_Iso);

            //if (getOperMode() != EOperMode_Meta)
            rDbs.setEnable(MTRUE);

            // Get default NVRAM parameter
            ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS(m_ParamIdx_P1.DBS);

            //Interpolation ISP module
            if (m_IspInterpCtrl
                &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
            {
                  if ( temp_CamInfo.eIdx_ISO_L != temp_CamInfo.eIdx_ISO_U || m_rIspCamInfo.eIdx_Zoom_L != m_rIspCamInfo.eIdx_Zoom_U)
                  {
                       m_ISP_INT.sDbs.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).DBS;
                       m_ISP_INT.sDbs.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).DBS;
                       m_ISP_INT.sDbs.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).DBS;
                       m_ISP_INT.sDbs.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).DBS;


                      CAM_LOGD_IF(m_bDebugEnable,"%s(): customized DBS_Iso = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                                __FUNCTION__, DBS_Iso,
                                 m_ISP_INT.sDbs.u2UpperIso_UpperZoom,
                                 m_ISP_INT.sDbs.u2LowerIso_UpperZoom,
                                 m_ISP_INT.sDbs.u2UpperIso_LowerZoom,
                                 m_ISP_INT.sDbs.u2LowerIso_LowerZoom);

                      AAA_TRACE_G(SmoothDBS);
                      AAA_TRACE_NL(SmoothDBS);

                      SmoothDBS(DBS_Iso,  // Real ISO
                                m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_U], // Upper ISO
                                m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_L], // Lower ISO
                                m_IspNvramMgr.getDBS(m_ISP_INT.sDbs.u2UpperIso_LowerZoom), // DBS settings for upper ISO
                                m_IspNvramMgr.getDBS(m_ISP_INT.sDbs.u2LowerIso_LowerZoom), // DBS settings for lower ISO
                                dbs);  // Output

                      AAA_TRACE_END_G;
                      AAA_TRACE_END_NL;

                  }
            }

            // Invoke callback for customers to modify.
            if  ( isDynamicTuning() )
            {   //  Dynamic Tuning: Enable
                m_pIspTuningCustom->refine_DBS(m_rIspCamInfo, m_IspNvramMgr, dbs);
            }

            // Load it to ISP manager buffer.
            rDbs.put(dbs);
        }
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
    MBOOL bDisable = propertyGet("vendor.isp.obc.disable", 0);
    ISP_MGR_OBC_T& rObc = ISP_MGR_OBC_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgOBC = rObc.isCCTEnable();
    if ((bDisable) || (!rObc.isCCTEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_OBC_EN)) || m_DualPD_PureRaw)
    {
        rObc.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rObc.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC(m_ParamIdx_P1.OBC);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            && m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U || m_rIspCamInfo.eIdx_Zoom_L != m_rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sObc.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).OBC;
                   m_ISP_INT.sObc.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).OBC;
                   m_ISP_INT.sObc.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).OBC;
                   m_ISP_INT.sObc.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).OBC;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sObc.u2UpperIso_UpperZoom,
                             m_ISP_INT.sObc.u2LowerIso_UpperZoom,
                             m_ISP_INT.sObc.u2UpperIso_LowerZoom,
                             m_ISP_INT.sObc.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothOBC);
                  AAA_TRACE_NL(SmoothOBC);
                  SmoothOBC(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc.u2UpperIso_LowerZoom), // OBC settings for upper ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc.u2LowerIso_LowerZoom), // OBC settings for lower ISO
                            obc);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
    MBOOL bDisable = propertyGet("vendor.isp.bpc.disable", 0);
    ISP_MGR_BNR_T& rBnr = ISP_MGR_BNR_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgBPC = rBnr.isCCTBPCEnable();

    if ((bDisable) || (!rBnr.isCCTBPCEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_BPC_EN))|| m_DualPD_PureRaw)
    {
        rBnr.setBPCEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rBnr.setBPCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_BPC_T bpc = m_IspNvramMgr.getBPC(m_ParamIdx_P1.BNR_BPC);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U || m_rIspCamInfo.eIdx_Zoom_L != m_rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sBpc.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).BNR_BPC;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sBpc.u2UpperIso_UpperZoom,
                             m_ISP_INT.sBpc.u2LowerIso_UpperZoom,
                             m_ISP_INT.sBpc.u2UpperIso_LowerZoom,
                             m_ISP_INT.sBpc.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothBPC);
                  AAA_TRACE_NL(SmoothBPC);
                  SmoothBPC(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc.u2UpperIso_LowerZoom), // BPC settings for upper ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc.u2LowerIso_LowerZoom), // BPC settings for lower ISO
                            bpc);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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

    MBOOL bDisable = propertyGet("vendor.isp.nr1.disable", 0);
    ISP_MGR_BNR_T& rBnr = ISP_MGR_BNR_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgNR1 = rBnr.isCCTCTEnable();

    if ((bDisable) || (!rBnr.isCCTCTEnable()) ||
        (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_NR1_EN)) ||
        (m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) || m_DualPD_PureRaw)
    {
        rBnr.setCTEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            rBnr.setCTEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_NR1_T nr1 = m_IspNvramMgr.getNR1(m_ParamIdx_P1.BNR_NR1);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U || m_rIspCamInfo.eIdx_Zoom_L != m_rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sNr1.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr1.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr1.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr1.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).BNR_NR1;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sNr1.u2UpperIso_UpperZoom,
                             m_ISP_INT.sNr1.u2LowerIso_UpperZoom,
                             m_ISP_INT.sNr1.u2UpperIso_LowerZoom,
                             m_ISP_INT.sNr1.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothNR1);
                  AAA_TRACE_NL(SmoothNR1);
                  SmoothNR1(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr1.u2UpperIso_LowerZoom), // NR1 settings for upper ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr1.u2LowerIso_LowerZoom), // NR1 settings for lower ISO
                            nr1);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
    MBOOL bDisable = propertyGet("vendor.isp.pdc.disable", 0);
    ISP_MGR_BNR_T& rBnr = ISP_MGR_BNR_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgPDC = rBnr.isCCTPDCEnable();

    if( (bDisable) ||
        (!rBnr.isCCTPDCEnable())  ||
         m_DualPD_PureRaw )
    {
        rBnr.setPDCEnable(MFALSE);
    }
    else{

        //if (getOperMode() != EOperMode_Meta)
        rBnr.setPDCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_PDC_T pdc = m_IspNvramMgr.getPDC(m_ParamIdx_P1.BNR_PDC);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U || m_rIspCamInfo.eIdx_Zoom_L != m_rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sPdc.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).BNR_PDC;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sPdc.u2UpperIso_UpperZoom,
                             m_ISP_INT.sPdc.u2LowerIso_UpperZoom,
                             m_ISP_INT.sPdc.u2UpperIso_LowerZoom,
                             m_ISP_INT.sPdc.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothPDC);
                  AAA_TRACE_NL(SmoothPDC);
                  SmoothPDC(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc.u2UpperIso_LowerZoom), // PDC settings for upper ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc.u2LowerIso_LowerZoom), // PDC settings for lower ISO
                            pdc);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
    MBOOL bDisable = propertyGet("vendor.isp.rmm.disable", 0);
    ISP_MGR_RMM_T& rRmm = ISP_MGR_RMM_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgRMM = rRmm.isCCTEnable();

    if ((bDisable) || (!rRmm.isCCTEnable()) ||
        (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_RMM_EN)))
    {
        rRmm.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rRmm.setEnable(MTRUE);

        // Get default NVRAM parameter
    ISP_NVRAM_RMM_T rmm = m_IspNvramMgr.getRMM(m_ParamIdx_P1.RMM);

    //Interpolation ISP module
    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
    {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U || m_rIspCamInfo.eIdx_Zoom_L != m_rIspCamInfo.eIdx_Zoom_U)
          {
                   m_ISP_INT.sRmm.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).RMM;
                   m_ISP_INT.sRmm.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).RMM;
                   m_ISP_INT.sRmm.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).RMM;
                   m_ISP_INT.sRmm.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).RMM;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                        __FUNCTION__, m_rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sRmm.u2UpperIso_UpperZoom,
                             m_ISP_INT.sRmm.u2LowerIso_UpperZoom,
                             m_ISP_INT.sRmm.u2UpperIso_LowerZoom,
                             m_ISP_INT.sRmm.u2LowerIso_LowerZoom);


              AAA_TRACE_G(SmoothRMM);
              AAA_TRACE_L(SmoothRMM);
              SmoothRMM(m_rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                        m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                        m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                        m_IspNvramMgr.getRMM(m_ISP_INT.sRmm.u2UpperIso_LowerZoom), // RMM settings for upper ISO
                        m_IspNvramMgr.getRMM(m_ISP_INT.sRmm.u2LowerIso_LowerZoom), // RMM settings for lower ISO
                        rmm);  // Output
              AAA_TRACE_END_G;
              AAA_TRACE_END_NL;
          }
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
    MBOOL bDisable = propertyGet("vendor.isp.rmg.disable", 0);
    ISP_MGR_RMG_T& rRmg = ISP_MGR_RMG_T::getInstance(m_eSensorDev);

    if ((bDisable) || (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_RMG_EN)))
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
    MBOOL bDisable = propertyGet("vendor.isp.rpg.disable", 0);
    ISP_MGR_RPG_T& rRpg = ISP_MGR_RPG_T::getInstance(m_eSensorDev);
    // Get default NVRAM parameter
    ISP_NVRAM_RPG_T rpg;

    AWB_GAIN_T rCurrentAWBGain = m_rIspCamInfo.rAWBInfo.rRPG;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] En(%d), AWB(%d,%d,%d)", __FUNCTION__,
        isRPGEnable(), rCurrentAWBGain.i4R, rCurrentAWBGain.i4G, rCurrentAWBGain.i4B);

    rRpg.setIspAWBGain(rCurrentAWBGain);

    if ((isRPGEnable() && (!bDisable)) && (!m_DualPD_PureRaw) &&
        (m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_RPGN_EN))
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
    MBOOL bDisable = propertyGet("vendor.isp.ccm.disable", 0);

    ISP_NVRAM_CCM_T rCCM;
    ::memset(&rCCM, 0, sizeof(ISP_NVRAM_CCM_T));

    if(m_rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX){
        rCCM = m_pCcmMgr->getCCM();
    }
    else if (isDynamicCCM() &&(!m_rIspCamInfo.rAWBInfo.bAWBLock))
    {
        if (m_rIspParam.ISPColorTbl.ISPMulitCCM.CCM_Method) { // smooth CCM
            MY_LOG_IF(m_bDebugEnable, "Smooth CCM");
            MBOOL bInvokeSmoothCCMwPrefGain = m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(m_rIspCamInfo);
            MY_LOG_IF(m_bDebugEnable, "is_to_invoke_smooth_ccm_with_preference_gain = %d", bInvokeSmoothCCMwPrefGain);
            AAA_TRACE_G(calculateCCM);
            AAA_TRACE_NL(calculateCCM);
            m_pCcmMgr->calculateCCM(rCCM, m_ISP_ALG_VER.sCCM ,bInvokeSmoothCCMwPrefGain, (m_u4RawFmtType == SENSOR_RAW_RWB ? MTRUE : MFALSE), m_rIspCamInfo);
            AAA_TRACE_END_NL;
            AAA_TRACE_END_G;
        }
        else { // dynamic CCM
            // Evaluate CCM index
            MY_LOG_IF(m_bDebugEnable, "Dynamic CCM");
            EIndex_CCM_T PrvIdx = (EIndex_CCM_T)m_ISP_DYM.sCcm_tbl_P1_Idx;
            m_ISP_DYM.sCcm_tbl_P1_Idx = (MUINT16)m_pIspTuningCustom->evaluate_CCM_index(m_rIspCamInfo, PrvIdx);
            rCCM = m_rIspParam.ISPColorTbl.ISPMulitCCM.dynamic_CCM[m_ISP_DYM.sCcm_tbl_P1_Idx];
        }
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
    MBOOL bDisable = propertyGet("vendor.isp.sl2g.disable", 1);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgSl2gEn = fgPureRaw && fgPgnEn;
    ISP_MGR_SL2G_T& rSl2g = ISP_MGR_SL2G_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgSl2gEn|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgSL2F)||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2FG_EN)))
    {
        rSl2g.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rSl2g.setEnable(MTRUE);

        //m_IspNvramMgr.setIdx_SL2F(rIspCamInfo.eSensorMode);
        ISP_NVRAM_SL2_T sl2g = m_IspNvramMgr.getSL2F(m_ParamIdx_P2.SL2F);

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
prepareHw_PerFrame_DBS_2(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bDisable = propertyGet("vendor.isp.dbs2.disable", 0);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgDbs2En = fgPureRaw && fgPgnEn;
    ISP_MGR_DBS2_T& rDbs2 = ISP_MGR_DBS2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgDbs2En|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgDBS)||
        (rIspCamInfo.i4P2InImgFmt) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_DBS_EN)))
    {
        rDbs2.setEnable(MFALSE);
    }
    else
    {
        if(!(rIspCamInfo.fgHDR)){

        //if (getOperMode() != EOperMode_Meta)
        rDbs2.setEnable(MTRUE);

        // Get default NVRAM parameter
            ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS(m_ParamIdx_P2.DBS);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sDbs2.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).DBS;
                   m_ISP_INT.sDbs2.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).DBS;
                   m_ISP_INT.sDbs2.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).DBS;
                   m_ISP_INT.sDbs2.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).DBS;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sDbs2.u2UpperIso_UpperZoom,
                             m_ISP_INT.sDbs2.u2LowerIso_UpperZoom,
                             m_ISP_INT.sDbs2.u2UpperIso_LowerZoom,
                             m_ISP_INT.sDbs2.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothDBS_2);
                  AAA_TRACE_NL(SmoothDBS_2);
                  SmoothDBS(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs2.u2UpperIso_LowerZoom), // DBS settings for upper ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs2.u2LowerIso_LowerZoom), // DBS settings for lower ISO
                            dbs);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_DBS(rIspCamInfo, m_IspNvramMgr, dbs);
        }

        // Load it to ISP manager buffer.
        rDbs2.put(dbs);

    }
        // HDR customization flow
        else{
            MUINT32 DBS_Iso = m_pIspTuningCustom->get_HdrDbsNewISO(rIspCamInfo);

            EIndex_ISO_T temp_IDX = map_ISO_value2index(DBS_Iso);

            RAWIspCamInfo temp_CamInfo;

            get_ISOIdx_neighbor(temp_CamInfo, temp_IDX, DBS_Iso);

            //if (getOperMode() != EOperMode_Meta)
            rDbs2.setEnable(MTRUE);

            // Get default NVRAM parameter
            ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS(m_ParamIdx_P2.DBS);

            //Interpolation ISP module
            if (m_IspInterpCtrl
                &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
            {
                  if ( temp_CamInfo.eIdx_ISO_L != temp_CamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
                  {
                       m_ISP_INT.sDbs2.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).DBS;
                       m_ISP_INT.sDbs2.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_U, m_rIspCamInfo).DBS;
                       m_ISP_INT.sDbs2.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_U, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).DBS;
                       m_ISP_INT.sDbs2.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(temp_CamInfo.eIdx_ISO_L, m_rIspCamInfo.eIdx_Zoom_L, m_rIspCamInfo).DBS;


                      CAM_LOGD_IF(m_bDebugEnable,"%s(): customized DBS_Iso = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                                __FUNCTION__, DBS_Iso,
                                 m_ISP_INT.sDbs2.u2UpperIso_UpperZoom,
                                 m_ISP_INT.sDbs2.u2LowerIso_UpperZoom,
                                 m_ISP_INT.sDbs2.u2UpperIso_LowerZoom,
                                 m_ISP_INT.sDbs2.u2LowerIso_LowerZoom);

                      AAA_TRACE_G(SmoothDBS);
                      AAA_TRACE_NL(SmoothDBS);

                      SmoothDBS(DBS_Iso,  // Real ISO
                                m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_U], // Upper ISO
                                m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[temp_CamInfo.eIdx_ISO_L], // Lower ISO
                                m_IspNvramMgr.getDBS(m_ISP_INT.sDbs2.u2UpperIso_LowerZoom), // DBS settings for upper ISO
                                m_IspNvramMgr.getDBS(m_ISP_INT.sDbs2.u2LowerIso_LowerZoom), // DBS settings for lower ISO
                                dbs);  // Output

                      AAA_TRACE_END_G;
                      AAA_TRACE_END_NL;

                  }
            }

            // Invoke callback for customers to modify.
            if  ( isDynamicTuning() )
            {   //  Dynamic Tuning: Enable
                m_pIspTuningCustom->refine_DBS(m_rIspCamInfo, m_IspNvramMgr, dbs);
            }

            // Load it to ISP manager buffer.
            rDbs2.put(dbs);
        }
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
    MBOOL bDisable = propertyGet("vendor.isp.obc2.disable", 0);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgObc2En = fgPureRaw && fgPgnEn;
    ISP_MGR_OBC2_T& rObc2 = ISP_MGR_OBC2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if ( !fgObc2En || bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgOBC)||
        (rIspCamInfo.i4P2InImgFmt) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_OBC_EN)))
    {
        rObc2.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rObc2.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC(m_ParamIdx_P2.OBC);;
        rObc2.setIspAEGain(rIspCamInfo.rAEInfo.u4IspGain>>1);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sObc2.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).OBC;
                   m_ISP_INT.sObc2.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).OBC;
                   m_ISP_INT.sObc2.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).OBC;
                   m_ISP_INT.sObc2.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).OBC;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sObc2.u2UpperIso_UpperZoom,
                             m_ISP_INT.sObc2.u2LowerIso_UpperZoom,
                             m_ISP_INT.sObc2.u2UpperIso_LowerZoom,
                             m_ISP_INT.sObc2.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothOBC_2);
                  AAA_TRACE_NL(SmoothOBC_2);
                  SmoothOBC(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc2.u2UpperIso_LowerZoom), // OBC settings for upper ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc2.u2LowerIso_LowerZoom), // OBC settings for lower ISO
                            obc);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
    MBOOL bDisable = propertyGet("vendor.isp.bpc2.disable", 0);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgBpc2En = fgPureRaw && fgPgnEn;
    ISP_MGR_BNR2_T& rBnr2 = ISP_MGR_BNR2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgBpc2En|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgBPC)||
        (rIspCamInfo.i4P2InImgFmt) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_BPC_EN)))
    {
        rBnr2.setBPC2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setBPC2Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_BPC_T bpc = m_IspNvramMgr.getBPC(m_ParamIdx_P2.BNR_BPC);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sBpc2.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc2.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc2.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc2.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).BNR_BPC;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sBpc2.u2UpperIso_UpperZoom,
                             m_ISP_INT.sBpc2.u2LowerIso_UpperZoom,
                             m_ISP_INT.sBpc2.u2UpperIso_LowerZoom,
                             m_ISP_INT.sBpc2.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothBPC_2);
                  AAA_TRACE_NL(SmoothBPC_2);
                  SmoothBPC(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc2.u2UpperIso_LowerZoom), // BPC settings for upper ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc2.u2LowerIso_LowerZoom), // BPC settings for lower ISO
                            bpc);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
    MBOOL bDisable = propertyGet("vendor.isp.nr12.disable", 0);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgNr12En = fgPureRaw && fgPgnEn;
    ISP_MGR_BNR2_T& rBnr2 = ISP_MGR_BNR2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if ( !fgNr12En || bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgNR1) ||
        (rIspCamInfo.i4P2InImgFmt) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)&&(fgPgnEn)) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_NR1_EN)))
    {
        rBnr2.setCT2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setCT2Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_NR1_T nr1 = m_IspNvramMgr.getNR1(m_ParamIdx_P2.BNR_NR1);;

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sNr12.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr12.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr12.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr12.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).BNR_NR1;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sNr12.u2UpperIso_UpperZoom,
                             m_ISP_INT.sNr12.u2LowerIso_UpperZoom,
                             m_ISP_INT.sNr12.u2UpperIso_LowerZoom,
                             m_ISP_INT.sNr12.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothNR1_2);
                  AAA_TRACE_NL(SmoothNR1_2);
                  SmoothNR1(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr12.u2UpperIso_LowerZoom), // NR1 settings for upper ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr12.u2LowerIso_LowerZoom), // NR1 settings for lower ISO
                            nr1);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
    MBOOL bDisable = propertyGet("vendor.isp.pdc2.disable", 0);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgPdc2En = fgPureRaw && fgPgnEn;
    ISP_MGR_BNR2_T& rBnr2 = ISP_MGR_BNR2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgPdc2En|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgPDC)||
        (rIspCamInfo.i4P2InImgFmt) ||
        (rIspCamInfo.eIspProfile == EIspProfile_YUV_Reprocess))
    {
        rBnr2.setPDC2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setPDC2Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_PDC_T pdc = m_IspNvramMgr.getPDC(m_ParamIdx_P2.BNR_PDC);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                   m_ISP_INT.sPdc2.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc2.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc2.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc2.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).BNR_PDC;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                             m_ISP_INT.sPdc2.u2UpperIso_UpperZoom,
                             m_ISP_INT.sPdc2.u2LowerIso_UpperZoom,
                             m_ISP_INT.sPdc2.u2UpperIso_LowerZoom,
                             m_ISP_INT.sPdc2.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothPDC_2);
                  AAA_TRACE_NL(SmoothPDC_2);
                  SmoothPDC(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc2.u2UpperIso_LowerZoom), // PDC settings for upper ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc2.u2LowerIso_LowerZoom), // PDC settings for lower ISO
                            pdc);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
    MBOOL bDisable = propertyGet("vendor.isp.rmm2.disable", 0);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgRmm2En = fgPureRaw && fgPgnEn;
    ISP_MGR_RMM2_T& rRmm2 = ISP_MGR_RMM2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgRmm2En || bDisable || (!(rIspCamInfo.fgCCTInfo.Bits.fgRMM)) ||
        (rIspCamInfo.i4P2InImgFmt) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_RMM_EN)))
    {
        rRmm2.setEnable(MFALSE);
    }
    else
    {
            rRmm2.setEnable(MTRUE);

            // Get default NVRAM parameter
            ISP_NVRAM_RMM_T rmm = m_IspNvramMgr.getRMM(m_ParamIdx_P2.RMM);
            if (m_IspInterpCtrl
                &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
            {
                if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                     m_ISP_INT.sRmm2.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).RMM;
                     m_ISP_INT.sRmm2.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).RMM;
                     m_ISP_INT.sRmm2.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).RMM;
                     m_ISP_INT.sRmm2.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).RMM;

                    CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                               __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                               m_ISP_INT.sRmm2.u2UpperIso_UpperZoom,
                               m_ISP_INT.sRmm2.u2LowerIso_UpperZoom,
                               m_ISP_INT.sRmm2.u2UpperIso_LowerZoom,
                               m_ISP_INT.sRmm2.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothRMM_2);
                  AAA_TRACE_NL(SmoothRMM_2);
                  SmoothRMM(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                                m_IspNvramMgr.getRMM(m_ISP_INT.sRmm2.u2UpperIso_LowerZoom), // RMM settings for upper ISO
                                m_IspNvramMgr.getRMM(m_ISP_INT.sRmm2.u2LowerIso_LowerZoom), // RMM settings for lower ISO
                            rmm);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;
              }
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
    MBOOL bDisable = propertyGet("vendor.isp.rmg2.disable", 0);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgRmg2En = fgPureRaw && fgPgnEn;
    ISP_MGR_RMG2_T& rRmg2 = ISP_MGR_RMG2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgRmg2En || bDisable  ||
        (rIspCamInfo.i4P2InImgFmt) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_RMG_EN)))
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
prepareHw_PerFrame_LSC_2(const ISP_INFO_T& rIspInfo)
{
    MBOOL fgRet = MFALSE;
    MBOOL bDisable = propertyGet("vendor.isp.lsc2.disable", 0);
    MBOOL fgPgnEn = !rIspInfo.rCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspInfo.rCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    EIspProfile_T profile = rIspInfo.rCamInfo.eIspProfile;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] Profile = %d", __FUNCTION__, profile);
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
            if (m_pLscMgr->getRawSize((ESensorMode_T) rIspInfo.rCamInfo.eSensorMode, u4W, u4H))
            {
                CAM_LOGD_IF(m_bDebugEnable, "[%s] Process Lsc Data @(%d), Modify Size(%dx%d -> %dx%d), grid(%dx%d), HwRto(%d)", __FUNCTION__,
                    rIspInfo.rCamInfo.u4Id, rCfg.i4ImgWd, rCfg.i4ImgHt, u4W, u4H, rCfg.i4GridX, rCfg.i4GridY, rRsvd.u4HwRto);
                #if CAM3_LSC2_USE_GAIN
                ILscTbl rTbl(ILscTable::GAIN_FIXED, u4W, u4H, rCfg.i4GridX, rCfg.i4GridY);
                #else
                ILscTbl rTbl(ILscTable::HWTBL, u4W, u4H, rCfg.i4GridX, rCfg.i4GridY);
                #endif
                if (profile == EIspProfile_N3D_Denoise_toW)
                    rTbl.setRsvdData(ILscTbl::RsvdData(32));
                else
                    rTbl.setRsvdData(rRsvd);
                rTbl.setData(rIspInfo.rLscData.data()+sizeof(ILscTable::Config)+sizeof(ILscTbl::RsvdData), rTbl.getSize());
                m_pLscMgr->syncTbl(rTbl, ILscMgr::LSC_P2);

                fgRet = MTRUE;
            }
            else
            {
                CAM_LOGE("Fail to get RAW size!");
            }
        }
        fgLsc2En = (profile != EIspProfile_N3D_Denoise) && (profile != EIspProfile_N3D_Denoise_toGGM);
        CAM_LOGD_IF(m_bDebugEnable, "[%s] fgLsc2En = %d", __FUNCTION__, fgLsc2En);
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] force off(%d), lsc2(%d), pgn(%d), pureraw(%d), fgRet(%d)", __FUNCTION__, bDisable, fgLsc2En, fgPgnEn, fgPureRaw, fgRet);

    if (!fgLsc2En || bDisable || (fgRet == MFALSE)||
        (rIspInfo.rCamInfo.i4P2InImgFmt) ||
        (rIspInfo.rCamInfo.eIspProfile == EIspProfile_YUV_Reprocess) ||
        (rIspInfo.rCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_HWNR))
    {
        rLsc2.enableLsc(MFALSE);
    }
    else
    {
        CAM_LOGD("[%s] force off(%d), lsc2(%d), pgn(%d), pureraw(%d), fgRet(%d), eIspProfile(%d)", __FUNCTION__, bDisable, fgLsc2En, fgPgnEn, fgPureRaw, fgRet, rIspInfo.rCamInfo.eIspProfile);
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
    MUINT32 bDisable = propertyGet("vendor.isp.sl2.disable", 0);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2_EN)))
    {
        rSl2.setEnable(MFALSE);
    }
    else
    {
        rSl2.setEnable(MTRUE);
    }

//SL2B, NBC
    MUINT32 bDisableSL2B = propertyGet("vendor.isp.sl2b.disable", 0);

    if ((bDisableSL2B)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2B_EN)))
    {
        rSl2.setSL2BEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2BEnable(MTRUE);
    }

//SL2C, NBC2
    MUINT32 bDisableSL2C = propertyGet("vendor.isp.sl2c.disable", 0);

    if ((bDisableSL2C)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2C_EN)))
    {
        rSl2.setSL2CEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2CEnable(MTRUE);
    }

//SL2D, SEEE
    MUINT32 bDisableSL2D = propertyGet("vendor.isp.sl2d.disable", 0);

    if ((bDisableSL2D)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2D_EN)))
    {
        rSl2.setSL2DEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2DEnable(MTRUE);
    }

//SL2E, NR3D
    MUINT32 bDisableSL2E = propertyGet("vendor.isp.sl2e.disable", 0);

    if ((bDisableSL2E)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2E_EN)))
    {
        rSl2.setSL2EEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2EEnable(MTRUE);
    }

//SL2H, RNR
    MUINT32 bDisableSL2H = propertyGet("vendor.isp.sl2h.disable", 0);

    if ((bDisableSL2H)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2H_EN)))
    {
        rSl2.setSL2HEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2HEnable(MTRUE);
    }

//SL2I, HFG
    MUINT32 bDisableSL2I = propertyGet("vendor.isp.sl2i.disable", 0);

    if ((bDisableSL2I)||
        (!rSl2.isCCTEnable())     ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2I_EN)))
    {
        rSl2.setSL2IEnable(MFALSE);
    }
    else
    {
        rSl2.setSL2IEnable(MTRUE);
    }

    MINT32 dbg_enable = propertyGet("vendor.debug.lsc_mgr.nsl2", -1);
    MINT32 i4Sl2Case = (dbg_enable != -1) ? dbg_enable : isEnableSL2(m_eSensorDev);


    if ( rSl2.isEnable()     || rSl2.getSL2BOnOff() || rSl2.getSL2COnOff() || rSl2.getSL2DOnOff() ||
         rSl2.getSL2EOnOff() || rSl2.getSL2HOnOff() || rSl2.getSL2IOnOff())
    {
        switch (rIspCamInfo.eIspProfile)
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
            case EIspProfile_N3D_Denoise_toYUV:
            case EIspProfile_N3D_Denoise_toW:
                m_IspNvramMgr.setIdx_SL2(46);
                m_ParamIdx_P2.SL2 = 46;
                break;
/*            case EIspProfile_N3D_HighRes_toYUV:
                m_IspNvramMgr.setIdx_SL2(47);
                m_ParamIdx_P2.SL2 = 47;
                break;*/
            case EIspProfile_N3D_MFHR_Before_Blend:
                m_IspNvramMgr.setIdx_SL2(48);
                m_ParamIdx_P2.SL2 = 48;
                break;
            case EIspProfile_N3D_MFHR_Single:
            case EIspProfile_N3D_MFHR_After_Blend:
            case EIspProfile_zHDR_Capture_MFNR_Single:
            case EIspProfile_zHDR_Capture_MFNR_After_Blend:
                m_IspNvramMgr.setIdx_SL2(49);
                m_ParamIdx_P2.SL2 = 49;
                break;
            default:
                m_IspNvramMgr.setIdx_SL2(
                    rIspCamInfo.eSensorMode*SHADING_SUPPORT_CT_NUM+ (UINT16)m_pIspTuningCustom->evaluate_Shading_CCT_index(rIspCamInfo));
                m_ParamIdx_P2.SL2 = (
                    rIspCamInfo.eSensorMode*SHADING_SUPPORT_CT_NUM+ (UINT16)m_pIspTuningCustom->evaluate_Shading_CCT_index(rIspCamInfo));
        }

        // Get default NVRAM parameter
        ISP_NVRAM_SL2_T sl2 = m_IspNvramMgr.getSL2(m_ParamIdx_P2.SL2);

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
    MBOOL bDisable = propertyGet("vendor.isp.pgn.disable", 0);
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
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_RPGN_EN))) {// RPG is enabled
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
    MBOOL bDisable = propertyGet("vendor.isp.rnr.disable", 0);
    ISP_MGR_RNR_T& rRnr = ISP_MGR_RNR_T::getInstance(m_eSensorDev);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||(!rRnr.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_RNR_EN)))
    {
        rRnr.setEnable(MFALSE);
    }
    else{
        rRnr.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_RNR_T rnr = m_IspNvramMgr.getRNR(m_ParamIdx_P2.RNR);


        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
            if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                 m_ISP_INT.sRnr.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).RNR;
                 m_ISP_INT.sRnr.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).RNR;
                 m_ISP_INT.sRnr.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).RNR;
                 m_ISP_INT.sRnr.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).RNR;

                CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                           __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                           m_ISP_INT.sRnr.u2UpperIso_UpperZoom,
                           m_ISP_INT.sRnr.u2LowerIso_UpperZoom,
                           m_ISP_INT.sRnr.u2UpperIso_LowerZoom,
                           m_ISP_INT.sRnr.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothRNR);
                  AAA_TRACE_NL(SmoothRNR);
                  SmoothRNR(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                          m_IspNvramMgr.getRNR(m_ISP_INT.sRnr.u2UpperIso_LowerZoom), // RNR settings for upper ISO
                          m_IspNvramMgr.getRNR(m_ISP_INT.sRnr.u2LowerIso_LowerZoom), // RNR settings for lower ISO
                            rnr);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
        }

        // FG mode protection
        if (rIspCamInfo.fgRPGEnable){
            rnr.con3.bits.RNR_FL_MODE = 1;
        }
        else{
            rnr.con3.bits.RNR_FL_MODE = 0;
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_RNR(rIspCamInfo, m_IspNvramMgr, rnr);
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
    MBOOL bDisable = propertyGet("vendor.isp.udm.disable", 0);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    ISP_MGR_UDM_T& rUdm = ISP_MGR_UDM_T::getInstance(m_eSensorDev);

    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    // Get default NVRAM parameter
    ISP_NVRAM_UDM_T udm = m_IspNvramMgr.getUDM(m_ParamIdx_P2.UDM);

    //Interpolation ISP module
    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
    {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                  EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100,rIspCamInfo.eIdx_Zoom_L);

                  m_ISP_INT.sUdm.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).UDM;
                  m_ISP_INT.sUdm.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).UDM;
                  m_ISP_INT.sUdm.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).UDM;
                  m_ISP_INT.sUdm.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).UDM;

                 CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                            m_ISP_INT.sUdm.u2UpperIso_UpperZoom,
                            m_ISP_INT.sUdm.u2LowerIso_UpperZoom,
                            m_ISP_INT.sUdm.u2UpperIso_LowerZoom,
                            m_ISP_INT.sUdm.u2LowerIso_LowerZoom);

                 ISP_NVRAM_UDM_INT_T rInt_Param;
                 rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
                 rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
                 rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
                 rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
                 rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
                 rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
                 rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getUDM(m_ISP_INT.sUdm.u2UpperIso_UpperZoom);
                 rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getUDM(m_ISP_INT.sUdm.u2LowerIso_UpperZoom);
                 rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getUDM(m_ISP_INT.sUdm.u2UpperIso_LowerZoom);
                 rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getUDM(m_ISP_INT.sUdm.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothUDM);
                  AAA_TRACE_NL(SmoothUDM);
                  SmoothUDM(rInt_Param,
                            udm);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_UDM_EN))) // bypass UDM    //(m_u4RawFmtType == SENSOR_RAW_MONO)?
    {
        udm.intp_crs.bits.UDM_BYP = 1;
    }

    // Load it to ISP manager buffer.
    rUdm.put(udm);


    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_FLC(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bDisable = propertyGet("vendor.isp.flc.disable", 0);

    ISP_MGR_FLC_T& rFlc = ISP_MGR_FLC_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    ISP_NVRAM_FLC_T flc;

    MINT16 i2FlareOffset = rIspCamInfo.rAEInfo.i2FlareOffset;

    MINT16 i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - i2FlareOffset);

    rFlc.setFLC(i2FlareGain, (-1*i2FlareOffset));

    if ((!m_pIspTuningCustom->is_to_invoke_flc(rIspCamInfo)) ||
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
    MBOOL bDisable = propertyGet("vendor.isp.ccm.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_CCM_T::getInstance(m_eSensorDev).isCCTEnable())            ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_CCM_EN))) {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        ISP_NVRAM_CCM_T rCCM = rIspCamInfo.rMtkCCM;

        if(((rIspCamInfo.eColorCorrectionMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) && (!rIspCamInfo.rAWBInfo.bAWBLock)) ||
            (rIspCamInfo.eIspProfile == EIspProfile_N3D_Capture_toW)  ||
            (rIspCamInfo.eIspProfile == EIspProfile_N3D_Denoise_toW)  ||
            (rIspCamInfo.eIspProfile == EIspProfile_N3D_Preview_toW)  ||
            (rIspCamInfo.eIspProfile == EIspProfile_N3D_Video_toW))
        {
            // Dynamic CCM
            if (isDynamicCCM())
            {
                if (m_rIspParam.ISPColorTbl.ISPMulitCCM.CCM_Method) { // smooth CCM
                    MY_LOG_IF(m_bDebugEnable, "Smooth CCM P2");
                    MY_LOG_IF(m_bDebugEnable, "is_to_invoke_smooth_ccm_with_preference_gain = %d", m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(rIspCamInfo));
                    MBOOL bInvokeSmoothCCMwPrefGain = m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(rIspCamInfo);
                    m_pCcmMgr->calculateCCM(rCCM, m_ISP_ALG_VER.sCCM, bInvokeSmoothCCMwPrefGain, (m_u4RawFmtType == SENSOR_RAW_RWB ? MTRUE : MFALSE), rIspCamInfo);

                }
                else { // dynamic CCM
                    // Evaluate CCM index
                    MY_LOG_IF(m_bDebugEnable, "Dynamic CCM P2");
                    EIndex_CCM_T ePrvIdx = (EIndex_CCM_T)m_ISP_DYM.sCcm_tbl_P2_Idx;
                    m_ISP_DYM.sCcm_tbl_P2_Idx = (MUINT16)m_pIspTuningCustom->evaluate_CCM_index(rIspCamInfo, ePrvIdx);
                    rCCM = m_rIspParam.ISPColorTbl.ISPMulitCCM.dynamic_CCM[m_ISP_DYM.sCcm_tbl_P2_Idx];
                }
                // Invoke callback for customers to modify.
                if  ( isDynamicTuning() )
                {   //  Dynamic Tuning: Enable
                    m_pIspTuningCustom->refine_CCM(rIspCamInfo, m_IspNvramMgr, rCCM);
                }
            }
        }

        // Load it to ISP manager buffer.
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).put(rCCM);
    }

    //set ISO to MDP
    MUINT32 FakeISO = (MUINT32)((((UINT64)rIspCamInfo.rAEInfo.u4AfeGain) * ((UINT64)rIspCamInfo.rAEInfo.u4IspGain) * 100 ) >> 20 );
    ISP_MGR_CCM_T::getInstance(m_eSensorDev).setISO(FakeISO);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bDisable = propertyGet("vendor.isp.ggm.disable", 0);

    ISP_NVRAM_GGM_T ggm;

    if(rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE){
        ggm = m_pGgmMgr->getGGM();
    }
    else
    {
        //dynamic ggm
        if (rIspCamInfo.fgHDR){
            ggm = m_rIspParam.ISPToneMap.IHDR_GGM[rIspCamInfo.rAEInfo.i4GammaIdx];
        }
        else {
            ggm = m_rIspParam.ISPToneMap.GGM[m_ParamIdx_P2.GGM];
        }

        //adaptive ggm
         AAA_TRACE_G(calculateGGM);
         AAA_TRACE_NL(calculateGGM);
         m_pGmaMgr->calculateGGM(&ggm, &m_GmaExifInfo, rIspCamInfo.rAEInfo, rIspCamInfo.eIspProfile);
         AAA_TRACE_END_G;
         AAA_TRACE_END_NL;

        // Invoke callback for customers to modify.
        if ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_GGM(rIspCamInfo,  m_IspNvramMgr, ggm);
            m_pIspTuningCustom->userSetting_EFFECT_GGM(rIspCamInfo, rIspCamInfo.eIdx_Effect, ggm);
        }
        m_pGgmMgr->updateGGM(ggm);
    }

    MBOOL CCToverwrite = propertyGet("vendor.cct.ggm.fake", 0);

    if(CCToverwrite){
        ggm = m_rIspParam.ISPToneMap.GGM[0];
    }

    ISP_MGR_GGM_T& rGgm = ISP_MGR_GGM_T::getInstance(m_eSensorDev);

    if ( (!rGgm.isCCTEnable()) || (bDisable)  ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_GGM_EN))) // GGM_LNR = 1
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
prepareHw_PerFrame_NDG(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bDisable = propertyGet("vendor.isp.ndg.disable", 0);
    ISP_MGR_NDG_T& rNdg = ISP_MGR_NDG_T::getInstance(m_eSensorDev);

    if ((!rNdg.isCCTEnable()) || (bDisable) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_NDG_EN)))
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
    MBOOL bDisable = propertyGet("vendor.isp.anr.disable", 0);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    ISP_MGR_NBC_T& rNbc = ISP_MGR_NBC_T::getInstance(m_eSensorDev);


    if ((bDisable) || (!rNbc.isCCTANR1Enable()) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)&&(!fgRPGEnable)) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_ANR_EN))
    )
    {
        rNbc.setANR1Enable(MFALSE);
    }
    else {

        rNbc.setANR1Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR(m_ParamIdx_P2.ANR);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                  EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100,rIspCamInfo.eIdx_Zoom_L);

                  m_ISP_INT.sAnr.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).ANR;
                  m_ISP_INT.sAnr.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).ANR;
                  m_ISP_INT.sAnr.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).ANR;
                  m_ISP_INT.sAnr.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).ANR;

                 CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                            m_ISP_INT.sAnr.u2UpperIso_UpperZoom,
                            m_ISP_INT.sAnr.u2LowerIso_UpperZoom,
                            m_ISP_INT.sAnr.u2UpperIso_LowerZoom,
                            m_ISP_INT.sAnr.u2LowerIso_LowerZoom);

                  ISP_NVRAM_ANR_INT_T rInt_Param;
                  rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
                  rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
                  rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
                  rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
                  rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
                  if (rIspCamInfo.eIdx_Zoom_L == eIDX_ZOOM_0){
                     rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[eIDX_ZOOM_OFF];
                  }
                  else{
                  rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
                  }
                  rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2UpperIso_UpperZoom);
                  rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2LowerIso_UpperZoom);
                  rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2UpperIso_LowerZoom);
                  rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothANR);
                  AAA_TRACE_NL(SmoothANR);
                  SmoothANR(rInt_Param, anr);
                  AAA_TRACE_END_NL;
                  AAA_TRACE_END_G;
              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_ANR(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2UpperIso_LowerZoom),
                                                  m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2LowerIso_LowerZoom), anr);
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
    MBOOL bDisable = propertyGet("vendor.isp.anr_tbl.disable", 0);

    ISP_MGR_NBC_T& rNbc = ISP_MGR_NBC_T::getInstance(m_eSensorDev);

    if(bDisable || !(rNbc.isANR1Enable())){
        rNbc.setANRTBLEnable(MFALSE);
    }
    else{
        rNbc.setANRTBLEnable(MTRUE);

        ISP_NVRAM_ANR_LUT_T anr_tbl;

        SmoothANR_TBL(rIspCamInfo, m_rIspParam.ISPColorTbl.ANR_TBL, anr_tbl);

#if 0
        // Evaluate ANR_TBL_CT index
        EIndex_ANR_TBL_CT_T const ePrvIdxCT = (EIndex_ANR_TBL_CT_T)m_ISP_DYM.sAnr_tbl_CT_Idx;
        EIndex_ANR_TBL_ISO_T const ePrvIdxISO = (EIndex_ANR_TBL_ISO_T)m_ISP_DYM.sAnr_tbl_ISO_Idx;
        MBOOL fgNeedInterISO = MFALSE;

        m_ISP_DYM.sAnr_tbl_CT_Idx = m_pIspTuningCustom->evaluate_ANR_TBL_CT_index(rIspCamInfo, ePrvIdxCT);
        m_ISP_DYM.sAnr_tbl_ISO_Idx = m_pIspTuningCustom->evaluate_ANR_TBL_ISO_index(rIspCamInfo, fgNeedInterISO, ePrvIdxISO);

        //interpolation ANR_TBL
        if(m_IspInterpCtrl
             &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
            m_ISP_INT.sAnrTblIso.u2UpperIso_LowerZoom = m_ISP_DYM.sAnr_tbl_ISO_Idx;
            m_ISP_INT.sAnrTblIso.u2LowerIso_LowerZoom = m_ISP_INT.sAnrTblIso.u2UpperIso_LowerZoom;

            if(fgNeedInterISO){
                m_ISP_INT.sAnrTblIso.u2LowerIso_LowerZoom = m_ISP_INT.sAnrTblIso.u2UpperIso_LowerZoom - 1;
                MUINT32* ANR_TBL_ISO_env = static_cast<MUINT32*>(m_pIspTuningCustom->get_ANR_TBL_ISO_IDX_info(m_eSensorDev));
                MUINT32 ANR_TBL_SIZE = m_pIspTuningCustom->get_ANR_TBL_SUPPORT_info();
                SmoothANR_TBL(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                             ANR_TBL_ISO_env[m_ISP_INT.sAnrTblIso.u2UpperIso_LowerZoom], // Upper ISO
                             ANR_TBL_ISO_env[m_ISP_INT.sAnrTblIso.u2LowerIso_LowerZoom], // Lower ISO
                             m_rIspParam.ISPColorTbl.ANR_TBL[m_ISP_DYM.sAnr_tbl_CT_Idx][m_ISP_INT.sAnrTblIso.u2UpperIso_LowerZoom], // ANR settings for upper ISO
                             m_rIspParam.ISPColorTbl.ANR_TBL[m_ISP_DYM.sAnr_tbl_CT_Idx][m_ISP_INT.sAnrTblIso.u2LowerIso_LowerZoom], // ANR settings for lower ISO
                             anr_tbl);  // Output
            }
        }
#endif
        rNbc.putANR_TBL(anr_tbl);
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
    MBOOL bDisable = propertyGet("vendor.isp.ndg2.disable", 0);
    ISP_MGR_NDG2_T& rNdg2 = ISP_MGR_NDG2_T::getInstance(m_eSensorDev);

    if ((!rNdg2.isCCTEnable()) || (bDisable) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_NDG2_EN)))
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
    MBOOL bDisable = propertyGet("vendor.isp.anr2.disable", 0);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||
        (!rNbc2.isCCTANR2Enable()) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)&&(!fgRPGEnable)) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL)       ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_ANR2_EN)))
    {

        rNbc2.setANR2Enable(MFALSE);
    }

    else {
        rNbc2.setANR2Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_ANR2_T anr2 = m_IspNvramMgr.getANR2(m_ParamIdx_P2.ANR2);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                  EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100,rIspCamInfo.eIdx_Zoom_L);

                  m_ISP_INT.sAnr2.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).ANR2;
                  m_ISP_INT.sAnr2.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).ANR2;
                  m_ISP_INT.sAnr2.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).ANR2;
                  m_ISP_INT.sAnr2.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).ANR2;

                 CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                            m_ISP_INT.sAnr2.u2UpperIso_UpperZoom,
                            m_ISP_INT.sAnr2.u2LowerIso_UpperZoom,
                            m_ISP_INT.sAnr2.u2UpperIso_LowerZoom,
                            m_ISP_INT.sAnr2.u2LowerIso_LowerZoom);

                 ISP_NVRAM_ANR2_INT_T rInt_Param;
                 rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
                 rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
                 rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
                 rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
                 rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
                 if (rIspCamInfo.eIdx_Zoom_L == eIDX_ZOOM_0)
                     rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[eIDX_ZOOM_OFF];
                 else
                 rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
                 rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2UpperIso_UpperZoom);
                 rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2LowerIso_UpperZoom);
                 rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2UpperIso_LowerZoom);
                 rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothANR2);
                  AAA_TRACE_NL(SmoothANR2);
                  SmoothANR2(rInt_Param, anr2);  // Output
                  AAA_TRACE_END_NL;
                  AAA_TRACE_END_G;
              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_ANR2(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2UpperIso_LowerZoom),
                                                  m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2LowerIso_LowerZoom), anr2);
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
    MBOOL bDisable = propertyGet("vendor.isp.ccr.disable", 0);

    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||
        (!rNbc2.isCCTCCREnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_CCR_EN))) {
        rNbc2.setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rNbc2.setCCREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR(m_ParamIdx_P2.CCR);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                  m_ISP_INT.sCcr.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).CCR;
                  m_ISP_INT.sCcr.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).CCR;
                  m_ISP_INT.sCcr.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).CCR;
                  m_ISP_INT.sCcr.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_L, rIspCamInfo).CCR;

                 CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                            m_ISP_INT.sCcr.u2UpperIso_UpperZoom,
                            m_ISP_INT.sCcr.u2LowerIso_UpperZoom,
                            m_ISP_INT.sCcr.u2UpperIso_LowerZoom,
                            m_ISP_INT.sCcr.u2LowerIso_LowerZoom);
                 MUINT16 u2LowerIso_LowerZoom;
                 MUINT16 u2LowerIso_UpperZoom;
                 MUINT16 u2UpperIso_LowerZoom;
                 MUINT16 u2UpperIso_UpperZoom;

                  AAA_TRACE_G(SmoothCCR);
                  AAA_TRACE_NL(SmoothCCR);
                  SmoothCCR(rIspCamInfo.rAEInfo.u4RealISOValue,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getCCR(m_ISP_INT.sCcr.u2UpperIso_LowerZoom), // CCR settings for upper ISO
                            m_IspNvramMgr.getCCR(m_ISP_INT.sCcr.u2LowerIso_LowerZoom), // CCR settings for lower ISO
                            ccr);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
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
#if 1
    MBOOL bDisable = propertyGet("vendor.isp.bok.disable", 1);
    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        bDisable = MTRUE;
    }

    if ((bDisable) ||(!rNbc2.isCCTBOKEnable()) ||
        (rIspCamInfo.eIspProfile == EIspProfile_YUV_Reprocess))
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
prepareHw_PerFrame_PCA(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bDisable = propertyGet("vendor.isp.pca.disable", 0);
    ISP_MGR_PCA_T& rPca = ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode);

//    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);
/*
    if (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
        m_ePCAMode = EPCAMode_360BIN;
    }
    else {
        m_ePCAMode = EPCAMode_180BIN;
    }
*/
    m_ePCAMode = EPCAMode_180BIN;

    if ((bDisable) || (!rPca.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_PCA_EN)))
    {
        rPca.setEnable(MFALSE);

        return  MTRUE;
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rPca.setEnable(MTRUE);

        //dynamic color
        ISP_NVRAM_PCA_LUT_T pca = m_rIspParam.ISPColorTbl.ISPColor.PCA_LUT[0][rIspCamInfo.eIdx_CT];

        MBOOL bIsCapture = m_pIspTuningCustom->is_to_invoke_capture_in_color(rIspCamInfo);

        if (m_rIspParam.ISPColorTbl.ISPColor.COLOR_Method && m_IspInterpCtrl)
        {// Smooth PCA
            SmoothPCA_TBL(  bIsCapture,
                            m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[rIspCamInfo.eIdx_CT_U],
                            m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[rIspCamInfo.eIdx_CT_L],
                            rIspCamInfo.eIdx_CT_U, rIspCamInfo.eIdx_CT_L,
                            m_rIspParam.ISPColorTbl.ISPColor.PCA_LUT,
                            m_rIspParam.ISPColorTbl.ISPColor.rPcaParam,
                            rIspCamInfo,
                            pca, m_ISP_ALG_VER.sPCA);
        }

        // (2) Invoke callback for customers to modify.
        if  (isDynamicTuning())
        {   // Dynamic Tuning: Enable
                   //m_pPcaMgr->setIdx(static_cast<MUINT32>(m_ISP_DYM.sPca_tbl_Idx));
        }
        m_pPcaMgr->loadConfig();
        rPca.loadLut(reinterpret_cast<MUINT32*>(&pca.lut[0]));

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
    MBOOL bDisable = propertyGet("vendor.isp.hfg.disable", 0);
    ISP_MGR_HFG_T& rHfg = ISP_MGR_HFG_T::getInstance(m_eSensorDev);

    if ((bDisable) ||(!rHfg.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_HFG_EN)))
    {
        rHfg.setEnable(MFALSE);
    }
    else{
        rHfg.setEnable(MTRUE);
        // Get default NVRAM parameter
        ISP_NVRAM_HFG_T hfg = m_IspNvramMgr.getHFG(m_ParamIdx_P2.HFG);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                  EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100,rIspCamInfo.eIdx_Zoom_L);

                  m_ISP_INT.sHfg.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).HFG;
                  m_ISP_INT.sHfg.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).HFG;
                  m_ISP_INT.sHfg.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).HFG;
                  m_ISP_INT.sHfg.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).HFG;

                 CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                            m_ISP_INT.sHfg.u2UpperIso_UpperZoom,
                            m_ISP_INT.sHfg.u2LowerIso_UpperZoom,
                            m_ISP_INT.sHfg.u2UpperIso_LowerZoom,
                            m_ISP_INT.sHfg.u2LowerIso_LowerZoom);

                ISP_NVRAM_HFG_INT_T rInt_Param;
                rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
                rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
                rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
                rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
                rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
                rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
                rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getHFG(m_ISP_INT.sHfg.u2UpperIso_UpperZoom);
                rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getHFG(m_ISP_INT.sHfg.u2LowerIso_UpperZoom);
                rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getHFG(m_ISP_INT.sHfg.u2UpperIso_LowerZoom);
                rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getHFG(m_ISP_INT.sHfg.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothHFG);
                  AAA_TRACE_NL(SmoothHFG);
                  SmoothHFG(rInt_Param, hfg);  // Output
                  AAA_TRACE_END_NL;
                  AAA_TRACE_END_G;
              }
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
    MBOOL bDisable = propertyGet("vendor.isp.ee.disable", 0);
    ISP_MGR_SEEE_T& rSeee = ISP_MGR_SEEE_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d), level %d mode %d", __FUNCTION__,
        fgRPGEnable, rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, rIspCamInfo.eEdgeMode);

    //EE
    if ((bDisable) || (!rSeee.isCCTEEEnable()) ||
        (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                   ||
        ((rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG)&&(!fgRPGEnable))||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_EE_EN)) )
    {
        rSeee.setEEEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rSeee.setEEEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE(m_ParamIdx_P2.EE);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
              {
                  EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100,rIspCamInfo.eIdx_Zoom_L);

                  m_ISP_INT.sEe.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).EE;
                  m_ISP_INT.sEe.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).EE;
                  m_ISP_INT.sEe.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).EE;
                  m_ISP_INT.sEe.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).EE;

                 CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d, zoom_transform(%d)\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                            m_ISP_INT.sEe.u2UpperIso_UpperZoom,
                            m_ISP_INT.sEe.u2LowerIso_UpperZoom,
                            m_ISP_INT.sEe.u2UpperIso_LowerZoom,
                            m_ISP_INT.sEe.u2LowerIso_LowerZoom,
                            zoom_transform);

                   ISP_NVRAM_EE_INT_T rInt_Param;
                   rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
                   rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
                   rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
                   rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
                   rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
                   if (rIspCamInfo.eIdx_Zoom_L == eIDX_ZOOM_0){
                     rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[eIDX_ZOOM_OFF];
                   }
                   else{
                   rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
                   }
                   rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2UpperIso_UpperZoom);
                   rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2LowerIso_UpperZoom);
                   rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2UpperIso_LowerZoom);
                   rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2LowerIso_LowerZoom);

                  AAA_TRACE_G(SmoothEE);
                  AAA_TRACE_NL(SmoothEE);
                  SmoothEE(rInt_Param, ee);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_EE(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2UpperIso_LowerZoom),
                                                  m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2LowerIso_LowerZoom), ee);
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
    MBOOL bDisable = propertyGet("vendor.isp.g2c.disable", 0);
    ISP_MGR_G2C_T& rG2c = ISP_MGR_G2C_T::getInstance(m_eSensorDev);
    ISP_MGR_SEEE_T& rSeee = ISP_MGR_SEEE_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] effect(%d)", __FUNCTION__, rIspCamInfo.eIdx_Effect);

    MBOOL bDisable2 = propertyGet("vendor.isp.se.disable", 0);

    //SE
    if (bDisable2 ||
        (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                   ||
        ((rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG)&&(!fgRPGEnable)) ||
        (rIspCamInfo.i4P2InImgFmt) ||
        (rIspCamInfo.eIdx_Effect == MTK_CONTROL_EFFECT_MODE_OFF))
    {
        rSeee.setSEEnable(MFALSE);
    }
    else
    {
        rSeee.setSEEnable(MTRUE);
    }

    //G2C
    if ((bDisable) || (!rG2c.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_G2C_EN))
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
    ISP_NVRAM_NR3D_T nr3d = m_IspNvramMgr.getNR3D(m_ParamIdx_P2.NR3D);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    ISP_NR3D_SMOOTH_INFO_T NR3D_SW_Params;

    NR3D_SW_Params.Device_ID = m_eSensorDev;
    NR3D_SW_Params.M_Info = rIspCamInfo.NR3D_Data;
    NR3D_SW_Params.C_Paras = m_rIspParam.ISPColorTbl.NR3D_PARA;

    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
    {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
          {
                  EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100, rIspCamInfo.eIdx_Zoom_L);

                  m_ISP_INT.sNr3d.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).NR3D;
                  m_ISP_INT.sNr3d.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).NR3D;
                  m_ISP_INT.sNr3d.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).NR3D;
                  m_ISP_INT.sNr3d.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).NR3D;

                 CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                            __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                            m_ISP_INT.sNr3d.u2UpperIso_UpperZoom,
                            m_ISP_INT.sNr3d.u2LowerIso_UpperZoom,
                            m_ISP_INT.sNr3d.u2UpperIso_LowerZoom,
                            m_ISP_INT.sNr3d.u2LowerIso_LowerZoom);

                ISP_NVRAM_NR3D_INT_T rInt_Param;
                rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
                rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
                rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
                rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
                rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
                rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
                rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getNR3D(m_ISP_INT.sNr3d.u2UpperIso_UpperZoom);
                rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getNR3D(m_ISP_INT.sNr3d.u2LowerIso_UpperZoom);
                rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getNR3D(m_ISP_INT.sNr3d.u2UpperIso_LowerZoom);
                rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getNR3D(m_ISP_INT.sNr3d.u2LowerIso_LowerZoom);

              SmoothNR3D(rInt_Param, nr3d, NR3D_SW_Params);  // Output

          }
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
    MBOOL bDisable = propertyGet("vendor.isp.color.disable", 0);
    ISP_MGR_NR3D_T& rNr3d = ISP_MGR_NR3D_T::getInstance(m_eSensorDev);
    CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.eColorCorrectionMode = %d",__FUNCTION__,rIspCamInfo.eColorCorrectionMode);

    if ((bDisable) || (!rNr3d.isCCTColorEnable()) || (rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_COLOR_EN)))
    {
        rNr3d.setColorEnable(MFALSE);

        return  MTRUE;
    }
    else{
        rNr3d.setColorEnable(MTRUE);

        //dynamic color
        ISP_NVRAM_COLOR_T color = m_rIspParam.ISPColorTbl.ISPColor.COLOR[0][rIspCamInfo.eIdx_CT];

        MBOOL bIsCapture = m_pIspTuningCustom->is_to_invoke_capture_in_color(rIspCamInfo);

        if( m_rIspParam.ISPColorTbl.ISPColor.COLOR_Method && m_IspInterpCtrl)
        {
                SmoothCOLOR( bIsCapture,
                             m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[rIspCamInfo.eIdx_CT_U],
                             m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[rIspCamInfo.eIdx_CT_L],
                             rIspCamInfo.eIdx_CT_U, rIspCamInfo.eIdx_CT_L,
                             m_rIspParam.ISPColorTbl.ISPColor.COLOR,
                             m_rIspParam.ISPColorTbl.ISPColor.rPcaParam,
                             rIspCamInfo,
                             color, m_ISP_ALG_VER.sCOLOR,
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
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB(m_ParamIdx_P2.MFB);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

     //Interpolation ISP module
     if (m_IspInterpCtrl
         &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
     {
           if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
           {
               EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100, rIspCamInfo.eIdx_Zoom_L);

               m_ISP_INT.sMfb.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).MFB;
               m_ISP_INT.sMfb.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).MFB;
               m_ISP_INT.sMfb.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).MFB;
               m_ISP_INT.sMfb.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).MFB;

              CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                         __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                         m_ISP_INT.sMfb.u2UpperIso_UpperZoom,
                         m_ISP_INT.sMfb.u2LowerIso_UpperZoom,
                         m_ISP_INT.sMfb.u2UpperIso_LowerZoom,
                         m_ISP_INT.sMfb.u2LowerIso_LowerZoom);

             ISP_NVRAM_MFB_INT_T rInt_Param;
             rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
             rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
             rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
             rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
             rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
             rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
             rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getMFB(m_ISP_INT.sMfb.u2UpperIso_UpperZoom);
             rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getMFB(m_ISP_INT.sMfb.u2LowerIso_UpperZoom);
             rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getMFB(m_ISP_INT.sMfb.u2UpperIso_LowerZoom);
             rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getMFB(m_ISP_INT.sMfb.u2LowerIso_LowerZoom);

             SmoothMFB(rInt_Param, mfb);  // Output
           }
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
    ISP_NVRAM_MIXER3_T mixer3 = m_IspNvramMgr.getMIX3(m_ParamIdx_P2.MIXER3);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    //Interpolation ISP module
    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
    {
          if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U || rIspCamInfo.eIdx_Zoom_L != rIspCamInfo.eIdx_Zoom_U)
          {
              EIndex_ZOOM_T zoom_transform = zoom_index_transform(rIspCamInfo.i4ZoomRatio_x100, rIspCamInfo.eIdx_Zoom_L);

              m_ISP_INT.sMix3.u2UpperIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).MIXER3;
              m_ISP_INT.sMix3.u2LowerIso_UpperZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo.eIdx_Zoom_U, rIspCamInfo).MIXER3;
              m_ISP_INT.sMix3.u2UpperIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, zoom_transform         , rIspCamInfo).MIXER3;
              m_ISP_INT.sMix3.u2LowerIso_LowerZoom = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, zoom_transform         , rIspCamInfo).MIXER3;

             CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso_UpperZoom = %d, u2LowerIso_UpperZoom = %d, u2UpperIso_LowerZoom = %d, u2LowerIso_LowerZoom = %d\n",
                        __FUNCTION__, rIspCamInfo.rAEInfo.u4RealISOValue,
                        m_ISP_INT.sMix3.u2UpperIso_UpperZoom,
                        m_ISP_INT.sMix3.u2LowerIso_UpperZoom,
                        m_ISP_INT.sMix3.u2UpperIso_LowerZoom,
                        m_ISP_INT.sMix3.u2LowerIso_LowerZoom);

            ISP_NVRAM_MIXER3_INT_T rInt_Param;
            rInt_Param.u4RealISO = rIspCamInfo.rAEInfo.u4RealISOValue;
            rInt_Param.u4UpperISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_U]; // Upper ISO
            rInt_Param.u4LowerISO = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[rIspCamInfo.eIdx_ISO_L]; // Lower ISO
            rInt_Param.i4ZoomRatio_x100 = rIspCamInfo.i4ZoomRatio_x100;
            rInt_Param.u4UpperZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_U];
            rInt_Param.u4LowerZoom = m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[rIspCamInfo.eIdx_Zoom_L];
            rInt_Param.rUpperIso_UpperZoom = m_IspNvramMgr.getMIX3(m_ISP_INT.sMix3.u2UpperIso_UpperZoom);
            rInt_Param.rLowerIso_UpperZoom = m_IspNvramMgr.getMIX3(m_ISP_INT.sMix3.u2LowerIso_UpperZoom);
            rInt_Param.rUpperIso_LowerZoom = m_IspNvramMgr.getMIX3(m_ISP_INT.sMix3.u2UpperIso_LowerZoom);
            rInt_Param.rLowerIso_LowerZoom = m_IspNvramMgr.getMIX3(m_ISP_INT.sMix3.u2LowerIso_LowerZoom);

            SmoothMIXER3(rInt_Param, mixer3);  // Output
          }
    }

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MIXER3(rIspCamInfo, m_IspNvramMgr, mixer3);
    }

    // Load it to ISP manager buffer
    ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).put(mixer3);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LCE(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bDisable = propertyGet("vendor.isp.lce.disable", 0);
    ISP_MGR_LCE_T& rLce = ISP_MGR_LCE_T::getInstance(m_eSensorDev);

    if ((bDisable) ||(!rLce.isCCTEnable()) || (m_pLCSBuffer == NULL) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_LCE_EN)) ||
        (rIspCamInfo.bBypassLCE) ||
        (rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) ||
        (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE) ||
        (rIspCamInfo.rLCS_Info.rOutSetting.u4OutWidth == 0) ||
        (rIspCamInfo.rLCS_Info.rOutSetting.u4OutHeight == 0))
    {
        rLce.setEnable(MFALSE);
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

        // Get Value from algo
        AAA_TRACE_G(calculateLCE);
        AAA_TRACE_NL(calculateLCE);
        m_pLceMgr->calculateLCE(rIspCamInfo.rLCS_Info.rOutSetting, &lce, &m_LceExifInfo, rIspCamInfo,
                                rIspCamInfo.rLCS_Info.rInSetting.lcs, pgn, ccm, (MUINT16*)m_pLCSBuffer->getImageBufferHeap()->getBufVA(0));

        AAA_TRACE_END_NL;
        AAA_TRACE_END_G;

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

    return  MTRUE;
}

