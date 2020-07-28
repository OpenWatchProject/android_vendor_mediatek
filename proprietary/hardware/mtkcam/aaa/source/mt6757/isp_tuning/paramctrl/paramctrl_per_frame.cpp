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
#include <pca_mgr.h>
#include <ispfeature.h>
#include <isp_interpolation.h>
#include <ccm_mgr.h>
#include <ggm_mgr.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <lsc/ILscMgr.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
//#include <hwutils/CameraProfile.h>
//#include "vfb_hal_base.h"
//#include <mtkcam/featureio/capturenr.h>
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

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
applyToHw_PerFrame_All(RequestSet_T const RequestSet, MINT32 i4SubsampleIdex)
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
    m_pTuning->dequeBuffer(RequestTuningSet.vNumberSet.data());

//if (RequestSet.vNumberSet[0] == 1)
{
    AAA_TRACE_G(applyToHw_perFrame_All);
    AAA_TRACE_NL(applyToHw_perFrame_All);

    for (MINT32 i4InitNum = 0; i4InitNum < m_i4SubsampleCount; i4InitNum++)
    {
        fgRet = ISP_MGR_AE_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_AWB_STAT_CONFIG::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_AF_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4InitNum)
            &&  ISP_MGR_PDO_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_FLK_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, m_bFlkEnable, i4InitNum)
            &&  ISP_MGR_SL2F_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.BinInfo, m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RMM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile, *m_pTuning, i4InitNum)
            &&  ISP_MGR_RMG_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum)
            ;
    }
    AAA_TRACE_END_NL;
    AAA_TRACE_END_G;

}
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

    localTimer.End();

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
applyToHw_PerFrame_P2(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pRegBuf)
{
    MBOOL fgRet = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] + pRegBuf(%p)", __FUNCTION__, pRegBuf);

    const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;
    dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);
    ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));

    AAA_TRACE_G(applyToHw_PerFrame_P2);
    fgRet = MTRUE
        &&  ISP_MGR_SL2G_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_DBS2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_OBC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_BNR2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_LSC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_SL2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspInfo.rIspP2CropInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_RNR_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_UDM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_LCE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_HFG_T::getInstance(m_eSensorDev).apply(rIspCamInfo.rCropRzInfo, rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        //&&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_RMM2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        &&  ISP_MGR_RMG2_T::getInstance(m_eSensorDev).apply(rIspCamInfo, pReg)
        ;

    AAA_TRACE_END_G;
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
        // SL2F should be after shading
        &&  prepareHw_PerFrame_SL2F()
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
        //pass1  LCS should be after CCM
        &&  prepareHw_PerFrame_LCS()
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
prepareHw_PerFrame_All(const ISP_INFO_T& rIspInfo, const IndexMgr& rIdx)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] +", __FUNCTION__);

    MBOOL fgRet = MTRUE
        &&  prepareHw_PerFrame_SL2G(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_DBS_2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_OBC_2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_BPC_2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_NR1_2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_PDC_2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_LSC_2(rIspInfo)
        &&  prepareHw_PerFrame_PGN(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_SL2(rIspInfo, rIdx)
        &&  prepareHw_PerFrame_RNR(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_UDM(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_CCM(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_GGM(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_ANR(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_ANR2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_CCR(rIspInfo.rCamInfo, rIdx)
        //&&  prepareHw_PerFrame_BOK(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_PCA(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_HFG(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_EE(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_EFFECT(rIspInfo.rCamInfo, rIdx)
        //&&  prepareHw_PerFrame_NR3D(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_MFB(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_MIXER3(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_LCE(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_RMM_2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_RMG_2(rIspInfo.rCamInfo, rIdx)
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
prepareHw_PerFrame_Partial(const ISP_INFO_T& rIspInfo, const IndexMgr& rIdx)
{
    prepareHw_PerFrame_UDM(rIspInfo.rCamInfo, rIdx);
    prepareHw_PerFrame_PGN(rIspInfo.rCamInfo, rIdx);
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

    MBOOL bSL2_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.SL2_EN;

    //Limit 2
    if (!bSL2_EN)
    {

        pReg->DIP_X_CTL_YUV_EN.Bits.SL2B_EN = 0;
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2C_EN = 0;
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2D_EN = 0;
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2E_EN = 0;
        pReg->DIP_X_CTL_RGB_EN.Bits.SL2H_EN = 0;
        pReg->DIP_X_CTL_YUV2_EN.Bits.SL2I_EN = 0;

        pReg->DIP_X_UDM_SL_CTL.Bits.UDM_SL_EN = 0;
    }

    MBOOL bNBC_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.NBC_EN;
    MBOOL bNBC2_EN = pReg->DIP_X_CTL_YUV_EN.Bits.NBC2_EN;
    MBOOL bSEEE_EN = pReg->DIP_X_CTL_YUV_EN.Bits.SEEE_EN;
    MBOOL bRNR_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.RNR_EN;
    MBOOL bHFG_EN  = pReg->DIP_X_CTL_YUV2_EN.Bits.HFG_EN;
    MBOOL bDBS2_EN = pReg->DIP_X_CTL_RGB_EN.Bits.DBS2_EN;
    //MBOOL bNR3D_EN = pReg->DIP_X_CTL_YUV_EN.Bits.NR3D_EN;

    //Limit 3
    if (!bNBC_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2B_EN = 0;
    }

    //Limit 4
    if (!bNBC2_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2C_EN = 0;
    }


    //Limit 5
    if (!bSEEE_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2D_EN = 0;
    }

    //Limit 6
    if (!bRNR_EN){
        pReg->DIP_X_CTL_RGB_EN.Bits.SL2H_EN = 0;
    }

    //Limit 7
    if (!bHFG_EN){
        pReg->DIP_X_CTL_YUV2_EN.Bits.SL2I_EN = 0;
    }

    //Limit 8
    if (!bDBS2_EN){
        pReg->DIP_X_CTL_RGB_EN.Bits.SL2G_EN = 0;
    }


/*
    //Limit 6
    if (!bNR3D_EN){
        pReg->DIP_X_CTL_YUV_EN.Bits.SL2E_EN = 0;
    }
*/


    MBOOL bSL2B_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2B_EN;
    MBOOL bSL2C_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2C_EN;
    MBOOL bSL2D_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2D_EN;
    MBOOL bSL2E_EN  = pReg->DIP_X_CTL_YUV_EN.Bits.SL2E_EN;
    MBOOL bSL2H_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.SL2H_EN;
    MBOOL bSL2I_EN  = pReg->DIP_X_CTL_YUV2_EN.Bits.SL2I_EN;

    MBOOL bSL2G_EN  = pReg->DIP_X_CTL_RGB_EN.Bits.SL2G_EN;


    //Limit 9
    if (bNBC_EN && (bSL2B_EN == 0) ){
        pReg->DIP_X_ANR_CON1.Bits.ANR_LCE_LINK = 0;
    }

    //Limit 10
    if (bNBC2_EN ){
        pReg->DIP_X_ANR2_CON1.Bits.ANR2_MODE = 0;
        if (bSL2C_EN == 0){
             pReg->DIP_X_ANR2_CON1.Bits.ANR2_LCE_LINK = 0;
        }
    }

    //Limit 11
    if (bSEEE_EN && (bSL2D_EN == 0) ){
        pReg->DIP_X_SEEE_SLNK_CTRL_2.Bits.SEEE_GLUT_LINK_EN = 0;
    }

/*
    //Limit 10
    if (bNR3D_EN && (bSL2E_EN == 0) ){
        pReg->DIP_X_NR3D_ON_CON.Bits.NR3D_SL2_OFF = 1;
    }
*/
    //Limit 12
    if (bRNR_EN && (bSL2H_EN == 0) ){
        pReg->DIP_X_RNR_SL.Bits.RNR_SL_EN = 0;
    }

    //Limit 13
    if (bHFG_EN && (bSL2I_EN == 0) ){
        pReg->DIP_X_CTL_YUV2_EN.Bits.HFG_EN = 0;
    }

    //Limit 14
    if (bDBS2_EN && (bSL2G_EN == 0) ){
        pReg->DIP_X_DBS2_CTL.Bits.DBS_SL_EN = 0;
    }

    MBOOL bPDC_EN = pReg->DIP_X_BNR2_PDC_CON.Bits.PDC_EN;
    // PDAF SW limitation
    if (bPDC_EN){
        pReg->DIP_X_BNR2_BPC_CON.Bits.BPC_LUT_EN = 1;
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

    MINT32 i4Rto = ::property_get_int32("vendor.debug.lsc_mgr.ratio", -1);

    MBOOL fgForceUnlock = (i4IsLockRto == IspTuningMgr::E_SHADING_RATIO_UNLOCK) && m_rIspCamInfo.rFlashInfo.isFlash;
    MBOOL fgLockRto = (m_rIspCamInfo.rAEInfo.bAELock || m_rIspCamInfo.rAEInfo.bAELimiter ||
                        (i4IsLockRto != IspTuningMgr::E_SHADING_RATIO_FREE_RUN)) && !fgForceUnlock;
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

    CAM_LOGD_IF(m_bDebugEnable, "[%s] (flash, iso, rto)=(%d, %d, %d), auto(%d), lock(%d,%d), dyn(%d), op(%d), PreCapFlashIsLockRto(%d), fgForceUnlock(%d)",
        __FUNCTION__, m_rIspCamInfo.rFlashInfo.isFlash, m_rIspCamInfo.rAEInfo.u4RealISOValue, i4Rto, fgAutoRto,
        m_rIspCamInfo.rAEInfo.bAELock, m_rIspCamInfo.rAEInfo.bAELimiter, m_fgDynamicShading, getOperMode(), i4IsLockRto, fgForceUnlock);

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

    //////////////////////////////////////
    MINT32 i4Tsf = ::property_get_int32("vendor.debug.lsc_mgr.manual_tsf", -1);
    MINT32 i4OnOff = ::property_get_int32("vendor.debug.lsc_mgr.enable", -1);

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
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2f.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
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
        m_IspNvramMgr.setIdx_SL2F(m_rIspCamInfo.eSensorMode);
        ISP_NVRAM_SL2_T sl2f = m_IspNvramMgr.getSL2F();

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
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.dbs.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_DBS_T& rDbs = ISP_MGR_DBS_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgDBS = rDbs.isCCTEnable();

    if ((bDisable) || (!rDbs.isCCTEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_DBS_EN))|| m_DualPD_PureRaw )
    {
        rDbs.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rDbs.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS();

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sDbs.u2UpperIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo).DBS;
                   m_ISP_INT.sDbs.u2LowerIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo).DBS;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sDbs.u2UpperIdx,
                            m_ISP_INT.sDbs.u2LowerIdx);

                  AAA_TRACE_G(SmoothDBS);
                  AAA_TRACE_NL(SmoothDBS);
                  SmoothDBS(m_rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs.u2UpperIdx), // DBS settings for upper ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs.u2LowerIdx), // DBS settings for lower ISO
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

    if ((bDisable) || (!rObc.isCCTEnable()) || (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_OBC_EN)) || m_DualPD_PureRaw)
    {
        rObc.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rObc.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC();

        //Interpolation ISP module
        if (m_IspInterpCtrl
            && m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sObc.u2UpperIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo).OBC;
                   m_ISP_INT.sObc.u2LowerIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo).OBC;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sObc.u2UpperIdx,
                            m_ISP_INT.sObc.u2LowerIdx);

                  AAA_TRACE_G(SmoothOBC);
                  AAA_TRACE_NL(SmoothOBC);
                  SmoothOBC(m_rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc.u2UpperIdx), // OBC settings for upper ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc.u2LowerIdx), // OBC settings for lower ISO
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
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.bpc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
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
        ISP_NVRAM_BNR_BPC_T bpc = m_IspNvramMgr.getBPC();

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sBpc.u2UpperIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc.u2LowerIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo).BNR_BPC;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sBpc.u2UpperIdx,
                            m_ISP_INT.sBpc.u2LowerIdx);

                  AAA_TRACE_G(SmoothBPC);
                  AAA_TRACE_NL(SmoothBPC);
                  SmoothBPC(m_rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc.u2UpperIdx), // BPC settings for upper ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc.u2LowerIdx), // BPC settings for lower ISO
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

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.nr1.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
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
        ISP_NVRAM_BNR_NR1_T nr1 = m_IspNvramMgr.getNR1();

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sNr1.u2UpperIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr1.u2LowerIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo).BNR_NR1;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sNr1.u2UpperIdx,
                            m_ISP_INT.sNr1.u2LowerIdx);

                  AAA_TRACE_G(SmoothNR1);
                  AAA_TRACE_NL(SmoothNR1);
                  SmoothNR1(m_rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr1.u2UpperIdx), // NR1 settings for upper ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr1.u2LowerIdx), // NR1 settings for lower ISO
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

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.pdc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_BNR_T& rBnr = ISP_MGR_BNR_T::getInstance(m_eSensorDev);

    m_rIspCamInfo.fgCCTInfo.Bits.fgPDC = rBnr.isCCTPDCEnable();

    if(( (rBnr.isPDCEnable()) && (rBnr.isCCTPDCEnable()))|| m_DualPD_PureRaw)
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr.setPDCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_PDC_T pdc = m_IspNvramMgr.getPDC();

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
        {
              if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sPdc.u2UpperIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc.u2LowerIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo).BNR_PDC;


                  MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sPdc.u2UpperIdx,
                            m_ISP_INT.sPdc.u2LowerIdx);

                  AAA_TRACE_G(SmoothPDC);
                  AAA_TRACE_NL(SmoothPDC);
                  SmoothPDC(m_rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc.u2UpperIdx), // PDC settings for upper ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc.u2LowerIdx), // PDC settings for lower ISO
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
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
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
    ISP_NVRAM_RMM_T rmm = m_IspNvramMgr.getRMM();

    //Interpolation ISP module
    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo))
    {
          if ( m_rIspCamInfo.eIdx_ISO_L != m_rIspCamInfo.eIdx_ISO_U)
          {
               m_ISP_INT.sRmm.u2UpperIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_U, m_rIspCamInfo).RMM;
               m_ISP_INT.sRmm.u2LowerIdx = getISPIndex_byISOIdx(m_rIspCamInfo.eIdx_ISO_L, m_rIspCamInfo).RMM;

              CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                        __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                        m_ISP_INT.sRmm.u2UpperIdx,
                        m_ISP_INT.sRmm.u2LowerIdx);

              AAA_TRACE_G(SmoothRMM);
              AAA_TRACE_NL(SmoothRMM);
              SmoothRMM(m_rIspCamInfo.u4ISOValue,  // Real ISO
                        m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_U], // Upper ISO
                        m_rIspIsoEnv.IDX_Partition[m_rIspCamInfo.eIdx_ISO_L], // Lower ISO
                        m_IspNvramMgr.getRMM(m_ISP_INT.sRmm.u2UpperIdx), // RMM settings for upper ISO
                        m_IspNvramMgr.getRMM(m_ISP_INT.sRmm.u2LowerIdx), // RMM settings for lower ISO
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

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RMG()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_RMG_T& rRmg = ISP_MGR_RMG_T::getInstance(m_eSensorDev);

    if ((bDisable) || (!(m_pFeatureCtrl[m_rIspCamInfo.eIspProfile] & M_RMG_EN)))
    {
        rRmg.setEnable(MFALSE);
    }
    else{
        rRmg.setEnable(MTRUE);
    }

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
    ISP_NVRAM_RPG_T rpg;

    AWB_GAIN_T rCurrentAWBGain = m_rIspCamInfo.rAWBInfo.rCurrentAWBGain;

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
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_CCM_INFO_T ccm_info;
    ::memset(&ccm_info, 0, sizeof(ISP_CCM_INFO_T));

    if(m_rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX){
        ccm_info.rCCM = m_pCcmMgr->getCCM();
    }
    else if (isDynamicCCM())
    {
        if (m_rIspParam.bInvokeSmoothCCM) { // smooth CCM
            MY_LOG_IF(m_bDebugEnable, "Smooth CCM");
            MY_LOG_IF(m_bDebugEnable, "is_to_invoke_smooth_ccm_with_preference_gain = %d", m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(m_rIspCamInfo));
            MBOOL bInvokeSmoothCCMwPrefGain = m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(m_rIspCamInfo);
            MINT32 i4SmoothMethod = m_pIspTuningCustom->get_CCM_smooth_method(m_rIspCamInfo);
            MINT32 i4ProjMethod = m_pIspTuningCustom->get_CCM_proj_method(m_rIspCamInfo);
            MINT32 i4ProjMax = m_pIspTuningCustom->get_CCM_proj_max(m_rIspCamInfo);
            AAA_TRACE_G(calculateCCM);
            AAA_TRACE_NL(calculateCCM);
            m_pCcmMgr->calculateCCM(ccm_info, m_rIspCamInfo.rAWBInfo, bInvokeSmoothCCMwPrefGain, m_i4FlashOnOff, i4SmoothMethod, m_rIspCamInfo.fgRWBSensor, i4ProjMethod, i4ProjMax);
            AAA_TRACE_END_G;
            AAA_TRACE_END_NL;
            if (m_rIspCamInfo.fgRWBSensor){
                m_pCcmMgr->RWB_CCM_decomposition(ccm_info.rCCM, m_rIspParam.isp_ccm_ratio);
            }
        }
        else { // dynamic CCM
            MY_LOG_IF(m_bDebugEnable, "Dynamic CCM");
            ccm_info.rCCM = m_rIspParam.ISPRegs.CCM[m_rIspCamInfo.eIdx_CCM];
        }
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm_info.rCCM);
        }
    }
    else {
        ccm_info.rCCM = m_pCcmMgr->getPrvCCM();//take output directly
    }

    m_pCcmMgr->updateCCM(ccm_info.rCCM);//for auto ccm result

    m_rIspCamInfo.rMtkCCMInfo = ccm_info;
    return  MTRUE;
        }

MBOOL
Paramctrl::
prepareHw_PerFrame_LCS(){


    ISP_MGR_LCS_T& rLcs = ISP_MGR_LCS_T::getInstance(m_eSensorDev);

    if(rLcs.isEnable()){

        ISP_NVRAM_RPG_T rpg;
        ISP_MGR_RPG_T::getInstance(m_eSensorDev).get(rpg);

        ISP_NVRAM_LCS_T lcs;
        ::memset(&lcs, 0, sizeof(ISP_NVRAM_LCS_T));
        rLcs.set_LCS(m_rIspCamInfo.rMtkCCMInfo.rCCM, rpg, lcs);

        rLcs.put(lcs);
    }

    rLcs.get(m_rIspCamInfo.rLcs_INFO.lcs);
    m_rIspCamInfo.rLcs_INFO.fgOnOff = rLcs.isEnable();

    return  MTRUE;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// prepare HW in pass 2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_SL2G(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2FG_EN)))
    {
        rSl2g.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rSl2g.setEnable(MTRUE);

        //m_IspNvramMgr.setIdx_SL2F(rIspCamInfo.eSensorMode);
        ISP_NVRAM_SL2_T sl2g = m_IspNvramMgr.getSL2F(rIspCamInfo.eSensorMode);

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
prepareHw_PerFrame_DBS_2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.dbs2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgDbs2En = fgPureRaw && fgPgnEn;
    ISP_MGR_DBS2_T& rDbs2 = ISP_MGR_DBS2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgDbs2En|| bDisable || (!rIspCamInfo.fgCCTInfo.Bits.fgDBS)||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_DBS_EN)))
    {
        rDbs2.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rDbs2.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS(rIdx.getIdx_DBS());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sDbs2.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).DBS;
                   m_ISP_INT.sDbs2.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).DBS;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sDbs2.u2UpperIdx,
                            m_ISP_INT.sDbs2.u2LowerIdx);

                  AAA_TRACE_G(SmoothDBS_2);
                  AAA_TRACE_NL(SmoothDBS_2);
                  SmoothDBS(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs2.u2UpperIdx), // DBS settings for upper ISO
                            m_IspNvramMgr.getDBS(m_ISP_INT.sDbs2.u2LowerIdx), // DBS settings for lower ISO
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

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_OBC_2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_OBC_EN)))
    {
        rObc2.setEnable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rObc2.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC(rIdx.getIdx_OBC());
        rObc2.setIspAEGain(rIspCamInfo.rAEInfo.u4IspGain>>1);

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sObc2.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).OBC;
                   m_ISP_INT.sObc2.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).OBC;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sObc2.u2UpperIdx,
                            m_ISP_INT.sObc2.u2LowerIdx);

                  AAA_TRACE_G(SmoothOBC_2);
                  AAA_TRACE_NL(SmoothOBC_2);

                  SmoothOBC(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc2.u2UpperIdx), // OBC settings for upper ISO
                            m_IspNvramMgr.getOBC(m_ISP_INT.sObc2.u2LowerIdx), // OBC settings for lower ISO
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
prepareHw_PerFrame_BPC_2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_BPC_EN)))
    {
        rBnr2.setBPC2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setBPC2Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_BPC_T bpc = m_IspNvramMgr.getBPC(rIdx.getIdx_BPC());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sBpc2.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).BNR_BPC;
                   m_ISP_INT.sBpc2.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).BNR_BPC;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sBpc2.u2UpperIdx,
                            m_ISP_INT.sBpc2.u2LowerIdx);

                  AAA_TRACE_G(SmoothBPC_2);
                  AAA_TRACE_NL(SmoothBPC_2);

                  SmoothBPC(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc2.u2UpperIdx), // BPC settings for upper ISO
                            m_IspNvramMgr.getBPC(m_ISP_INT.sBpc2.u2LowerIdx), // BPC settings for lower ISO
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
prepareHw_PerFrame_NR1_2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        ISP_NVRAM_BNR_NR1_T nr1 = m_IspNvramMgr.getNR1(rIdx.getIdx_NR1());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sNr12.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).BNR_NR1;
                   m_ISP_INT.sNr12.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).BNR_NR1;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sNr12.u2UpperIdx,
                            m_ISP_INT.sNr12.u2LowerIdx);

                  AAA_TRACE_G(SmoothNR1_2);
                  AAA_TRACE_NL(SmoothNR1_2);

                  SmoothNR1(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr12.u2UpperIdx), // NR1 settings for upper ISO
                            m_IspNvramMgr.getNR1(m_ISP_INT.sNr12.u2LowerIdx), // NR1 settings for lower ISO
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
prepareHw_PerFrame_PDC_2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        (rIspCamInfo.eIspProfile == EIspProfile_YUV_Reprocess))
    {
        rBnr2.setPDC2Enable(MFALSE);
    }
    else
    {
        //if (getOperMode() != EOperMode_Meta)
        rBnr2.setPDC2Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_PDC_T pdc = m_IspNvramMgr.getPDC(rIdx.getIdx_PDC());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sPdc2.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).BNR_PDC;
                   m_ISP_INT.sPdc2.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).BNR_PDC;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sPdc2.u2UpperIdx,
                            m_ISP_INT.sPdc2.u2LowerIdx);

                  AAA_TRACE_G(SmoothPDC_2);
                  AAA_TRACE_NL(SmoothPDC_2);

                  SmoothPDC(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc2.u2UpperIdx), // PDC settings for upper ISO
                            m_IspNvramMgr.getPDC(m_ISP_INT.sPdc2.u2LowerIdx), // PDC settings for lower ISO
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
prepareHw_PerFrame_RMM_2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmm2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgRmm2En = fgPureRaw && fgPgnEn;
    ISP_MGR_RMM2_T& rRmm2 = ISP_MGR_RMM2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgRmm2En || bDisable || (!(rIspCamInfo.fgCCTInfo.Bits.fgRMM)) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_RMM_EN)))
    {
        rRmm2.setEnable(MFALSE);
    }
    else
    {
            rRmm2.setEnable(MTRUE);

            // Get default NVRAM parameter
            ISP_NVRAM_RMM_T rmm = m_IspNvramMgr.getRMM(rIdx.getIdx_RMM());
            if (m_IspInterpCtrl
                &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
            {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sRmm2.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).RMM;
                   m_ISP_INT.sRmm2.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).RMM;

                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sRmm2.u2UpperIdx,
                            m_ISP_INT.sRmm2.u2LowerIdx);

                  AAA_TRACE_G(SmoothRMM_2);
                  AAA_TRACE_NL(SmoothRMM_2);

                  SmoothRMM(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getRMM(m_ISP_INT.sRmm2.u2UpperIdx), // RMM settings for upper ISO
                            m_IspNvramMgr.getRMM(m_ISP_INT.sRmm2.u2LowerIdx), // RMM settings for lower ISO
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
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RMG_2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rmg2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure;
    MBOOL fgRmg2En = fgPureRaw && fgPgnEn;
    ISP_MGR_RMG2_T& rRmg2 = ISP_MGR_RMG2_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    if (!fgRmg2En || bDisable  ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_RMG_EN)))
    {
        rRmg2.setEnable(MFALSE);
        }
    else
    {
        rRmg2.setEnable(MTRUE);
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
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] force off(%d), lsc2(%d), pgn(%d), pureraw(%d), fgRet(%d)", __FUNCTION__, bDisable, fgLsc2En, fgPgnEn, fgPureRaw, fgRet);

    if (!fgLsc2En || bDisable || (fgRet == MFALSE)||
        (rIspInfo.rCamInfo.eIspProfile == EIspProfile_YUV_Reprocess))
    {
        rLsc2.enableLsc(MFALSE);
    }
    else
    {
        rLsc2.enableLsc(MTRUE);
    }

    return MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_SL2(const ISP_INFO_T& rIspInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.sl2.disable", value, "0"); // 0: enable, 1: disable
    MUINT32 bDisable = atoi(value);
    MINT32 dbg_enable = ::property_get_int32("vendor.debug.lsc_mgr.nsl2", -1);
    MINT32 i4Sl2Case = (dbg_enable != -1) ? dbg_enable : isEnableSL2(m_eSensorDev);

    const NSIspTuning::RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;
    ISP_MGR_SL2_T& rSl2 = ISP_MGR_SL2_T::getInstance(m_eSensorDev);

    if ((bDisable)||
        (!rSl2.isCCTEnable())               ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_SL2_EN)))
    {
        rSl2.setEnable(MFALSE);
        rSl2.setSL2BEnable(MFALSE);
        rSl2.setSL2CEnable(MFALSE);
        rSl2.setSL2DEnable(MFALSE);
        rSl2.setSL2HEnable(MFALSE);
        rSl2.setSL2IEnable(MFALSE);
    }
    else
    {
        rSl2.setEnable(MTRUE);
        rSl2.setSL2BEnable(MTRUE);
        rSl2.setSL2CEnable(MTRUE);
        rSl2.setSL2DEnable(MTRUE);
        rSl2.setSL2HEnable(MTRUE);
        rSl2.setSL2IEnable(MTRUE);

        if ((rIspCamInfo.eIspProfile == EIspProfile_MFNR_After_Blend)  ||
            (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Capture_MFNR_After_Blend)  ||
            (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_HWNR)){

            m_IspNvramMgr.setIdx_SL2(NVRAM_SL2_TBL_NUM-1);
        }
        else{
            m_IspNvramMgr.setIdx_SL2(
                rIspCamInfo.eSensorMode*SHADING_SUPPORT_CT_NUM+ (UINT16)m_pIspTuningCustom->evaluate_Shading_CCT_index(rIspCamInfo));
        }
        // Get default NVRAM parameter
        ISP_NVRAM_SL2_T sl2 = m_IspNvramMgr.getSL2(rIdx.getIdx_SL2());

        if ( (i4Sl2Case == 1) &&
             (rIspCamInfo.eIspProfile != EIspProfile_MFNR_After_Blend)  &&
             (rIspCamInfo.eIspProfile != EIspProfile_zHDR_Capture_MFNR_After_Blend) &&
             (rIspCamInfo.eIspProfile != EIspProfile_Capture_MultiPass_HWNR))
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
prepareHw_PerFrame_PGN(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.pgn.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgPgnEnable = (!rIspCamInfo.fgRPGEnable);
    ISP_MGR_PGN_T& rPgn = ISP_MGR_PGN_T::getInstance(m_eSensorDev);

    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    AWB_GAIN_T rCurrentAWBGain = rIspCamInfo.rAWBInfo.rCurrentAWBGain;
    MINT16 i2FlareOffset = rIspCamInfo.rAEInfo.i2FlareOffset;
    MINT16 i2FlareGain = FLARE_SCALE_UNIT * FLARE_OFFSET_DOMAIN / (FLARE_OFFSET_DOMAIN - i2FlareOffset);

    CAM_LOGD_IF(m_bDebugEnable, "[%s] En(%d), AWB(%d,%d,%d), AE(%d,%d) \n", __FUNCTION__,
            fgPgnEnable, rCurrentAWBGain.i4R, rCurrentAWBGain.i4G, rCurrentAWBGain.i4B,i2FlareOffset,i2FlareGain );

    ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspAWBGain(rCurrentAWBGain);
    ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspFlare(i2FlareGain, (-1*i2FlareOffset));
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
prepareHw_PerFrame_RNR(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rnr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_RNR_T& rRnr = ISP_MGR_RNR_T::getInstance(m_eSensorDev);

    if ((bDisable) ||(!rRnr.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_RNR_EN)))
    {
        rRnr.setEnable(MFALSE);
    }
    else{
        rRnr.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_RNR_T rnr = m_IspNvramMgr.getRNR(rIdx.getIdx_RNR());


        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sRnr.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).RNR;
                   m_ISP_INT.sRnr.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).RNR;


                  MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sRnr.u2UpperIdx,
                            m_ISP_INT.sRnr.u2LowerIdx);

                  AAA_TRACE_G(SmoothRNR);
                  AAA_TRACE_NL(SmoothRNR);

                  SmoothRNR(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getRNR(m_ISP_INT.sRnr.u2UpperIdx), // RNR settings for upper ISO
                            m_IspNvramMgr.getRNR(m_ISP_INT.sRnr.u2LowerIdx), // RNR settings for lower ISO
                            rnr);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;


              }
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
prepareHw_PerFrame_UDM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.udm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    ISP_MGR_UDM_T& rUdm = ISP_MGR_UDM_T::getInstance(m_eSensorDev);

    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    // Get default NVRAM parameter
    ISP_NVRAM_UDM_T udm = m_IspNvramMgr.getUDM(rIdx.getIdx_UDM());

    //Interpolation ISP module
    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
    {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sUdm.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).UDM;
                   m_ISP_INT.sUdm.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).UDM;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sUdm.u2UpperIdx,
                            m_ISP_INT.sUdm.u2LowerIdx);

                  AAA_TRACE_G(SmoothUDM);
                  AAA_TRACE_NL(SmoothUDM);

                  SmoothUDM(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getUDM(m_ISP_INT.sUdm.u2UpperIdx), // UDM settings for upper ISO
                            m_IspNvramMgr.getUDM(m_ISP_INT.sUdm.u2LowerIdx), // UDM settings for lower ISO
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
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_UDM_EN))) // bypass UDM    //(m_bMono)?
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
prepareHw_PerFrame_CCM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    property_get("vendor.isp.ccm.supportmute", value, "0");
    MBOOL bSupportMute = atoi(value);

    MBOOL fgBlackMute = rIspCamInfo.fgBlackMute && bSupportMute;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] Mute(%d)", __FUNCTION__, fgBlackMute);

    if (m_bMono)
    {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setMono(MTRUE);
    }

    if ((bDisable) ||
        (!ISP_MGR_CCM_T::getInstance(m_eSensorDev).isCCTEnable())            ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_CCM_EN))) {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        ISP_CCM_INFO_T ccm_info = rIspCamInfo.rMtkCCMInfo;

        if((rIspCamInfo.fgNeedKeepP1) &&
           (rIspCamInfo.eColorCorrectionMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX))
        {
            // Dynamic CCM
            if (isDynamicCCM())
            {
                if (m_rIspParam.bInvokeSmoothCCM) { // smooth CCM
                    MY_LOG_IF(m_bDebugEnable, "Smooth CCM");
                    MY_LOG_IF(m_bDebugEnable, "is_to_invoke_smooth_ccm_with_preference_gain = %d", m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(rIspCamInfo));
                    MBOOL bInvokeSmoothCCMwPrefGain = m_pIspTuningCustom->is_to_invoke_smooth_ccm_with_preference_gain(rIspCamInfo);
                    MINT32 i4SmoothMethod = m_pIspTuningCustom->get_CCM_smooth_method(rIspCamInfo);
                    MINT32 i4ProjMethod = m_pIspTuningCustom->get_CCM_proj_method(rIspCamInfo);
                    MINT32 i4ProjMax = m_pIspTuningCustom->get_CCM_proj_max(rIspCamInfo);
                    AAA_TRACE_G(calculateCCM);
                    AAA_TRACE_NL(calculateCCM);
                    m_pCcmMgr->calculateCCM(ccm_info, rIspCamInfo.rAWBInfo, bInvokeSmoothCCMwPrefGain, m_i4FlashOnOff, i4SmoothMethod, rIspCamInfo.fgRWBSensor, i4ProjMethod, i4ProjMax);
                    AAA_TRACE_END_G;
                    AAA_TRACE_END_NL;

                    if (rIspCamInfo.fgRWBSensor){
                        m_pCcmMgr->RWB_CCM_decomposition(ccm_info.rCCM, m_rIspParam.isp_ccm_ratio);
                    }
                }
                else { // dynamic CCM
                    MY_LOG_IF(m_bDebugEnable, "Dynamic CCM");
                    ccm_info.rCCM = m_rIspParam.ISPRegs.CCM[rIspCamInfo.eIdx_CCM];
                }
                // Invoke callback for customers to modify.
                if  ( isDynamicTuning() )
                {   //  Dynamic Tuning: Enable
                    m_pIspTuningCustom->refine_CCM(rIspCamInfo, m_IspNvramMgr, ccm_info.rCCM);
                }
            }
        }

        if (fgBlackMute)
        {
            ::memset(&ccm_info.rCCM, 0, sizeof(ISP_NVRAM_CCM_T));
        }

        // Load it to ISP manager buffer.
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).put(ccm_info.rCCM);
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).updateCCMWeight(ccm_info.rCCMWeight);//for EXIF sync

    }
    //set ISO to MDP
    ISP_MGR_CCM_T::getInstance(m_eSensorDev).setISO(rIspCamInfo.u4ISOValue);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
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
        //dynamic ggm
        if ((rIspCamInfo.eIspProfile == EIspProfile_iHDR_Preview) ||
           (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Preview)                     ||
           (rIspCamInfo.eIspProfile == EIspProfile_mHDR_Preview)                     ||
            (rIspCamInfo.eIspProfile == EIspProfile_iHDR_Video)   ||
            (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Video)   ||
           (rIspCamInfo.eIspProfile == EIspProfile_mHDR_Video)                       ||
           (rIspCamInfo.eIspProfile == EIspProfile_iHDR_Preview_VSS)                 ||
           (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Preview_VSS)                 ||
           (rIspCamInfo.eIspProfile == EIspProfile_mHDR_Preview_VSS)                 ||
           (rIspCamInfo.eIspProfile == EIspProfile_iHDR_Video_VSS)                   ||
           (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Video_VSS)                   ||
           (rIspCamInfo.eIspProfile == EIspProfile_mHDR_Video_VSS)                   ||
            (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Capture) ||
           (rIspCamInfo.eIspProfile == EIspProfile_mHDR_Capture)                     ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_iHDR_Preview)                ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_zHDR_Preview)                ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_mHDR_Preview)                ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_iHDR_Video)                  ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_zHDR_Video)                  ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_mHDR_Video)                  ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_iHDR_Preview_VSS)            ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_zHDR_Preview_VSS)            ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_mHDR_Preview_VSS)            ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_iHDR_Video_VSS)              ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_zHDR_Video_VSS)              ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_mHDR_Video_VSS)              ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_zHDR_Capture)                ||
           (rIspCamInfo.eIspProfile == EIspProfile_Auto_mHDR_Capture)                ||
           (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Capture_MFNR_Before_Blend)   ||
           (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Capture_MFNR_Single)         ||
           (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Capture_MFNR_MFB)            ||
           (rIspCamInfo.eIspProfile == EIspProfile_zHDR_Capture_MFNR_After_Blend)    ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_iHDR_Preview)                 ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_zHDR_Preview)                 ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_mHDR_Preview)                 ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_iHDR_Video)                   ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_zHDR_Video)                   ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_mHDR_Video)                   ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_Auto_iHDR_Preview)            ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_Auto_zHDR_Preview)            ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_Auto_mHDR_Preview)            ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_Auto_iHDR_Video)              ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_Auto_zHDR_Video)              ||
           (rIspCamInfo.eIspProfile == EIspProfile_EIS_Auto_mHDR_Video)){
            ggm = m_IspNvramMgr.getIHDRGGM(rIspCamInfo.rAEInfo.i4GammaIdx);
        }
        else {
            ggm = m_IspNvramMgr.getGGM();
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
prepareHw_PerFrame_ANR(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.anr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
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
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR(rIdx.getIdx_ANR());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sAnr.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).ANR;
                   m_ISP_INT.sAnr.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).ANR;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sAnr.u2UpperIdx,
                            m_ISP_INT.sAnr.u2LowerIdx);

                  AAA_TRACE_G(SmoothANR);
                  AAA_TRACE_NL(SmoothANR);

                  SmoothANR(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2UpperIdx), // ANR settings for upper ISO
                            m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2LowerIdx), // ANR settings for lower ISO
                            anr);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;

              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_ANR(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2UpperIdx),
                                                  m_IspNvramMgr.getANR(m_ISP_INT.sAnr.u2LowerIdx), anr);
            }
            m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
        }

        // set ANR_LCE_LINK
#warning "SWNR linker error"
#if 0
        SwNRParam::getInstance(m_i4SensorIdx)->setANR_LCE_LINK(static_cast<MBOOL>(anr.con1.bits.ANR_LCE_LINK));
        m_pLscMgr->setSwNr();
#endif
        // Load it to ISP manager buffer.
        //rNbc.put(anr);



        char value2[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.anr_tbl.disable", value2, "0"); // 0: enable, 1: disable
        rNbc.setANRTBLEnable(!(atoi(value2)));

        ISP_NVRAM_ANR_LUT_T anr_tbl = m_rIspParam.ANR_TBL[rIspCamInfo.eIdx_ANR_TBL_CT][rIspCamInfo.eIdx_ANR_TBL_ISO];

        if (m_IspInterpCtrl
             &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
               MUINT32* ANR_TBL_ISO_env = static_cast<MUINT32*>(m_pIspTuningCustom->get_ANR_TBL_ISO_IDX_info(m_eSensorDev));
               MUINT32 ANR_TBL_SIZE = m_pIspTuningCustom->get_ANR_TBL_SUPPORT_info();

               if ( ((int)rIspCamInfo.eIdx_ANR_TBL_ISO < (ANR_TBL_SIZE-1))
                     && ((int)rIspCamInfo.eIdx_ANR_TBL_ISO > 0))
               {
                       AAA_TRACE_G(SmoothANR_TBL);
                    AAA_TRACE_NL(SmoothANR_TBL);
                   SmoothANR_TBL(rIspCamInfo.u4ISOValue,  // Real ISO
                             ANR_TBL_ISO_env[rIspCamInfo.eIdx_ANR_TBL_ISO], // Upper ISO
                             ANR_TBL_ISO_env[rIspCamInfo.eIdx_ANR_TBL_ISO-1], // Lower ISO
                             m_rIspParam.ANR_TBL[rIspCamInfo.eIdx_ANR_TBL_CT][rIspCamInfo.eIdx_ANR_TBL_ISO], // ANR settings for upper ISO
                             m_rIspParam.ANR_TBL[rIspCamInfo.eIdx_ANR_TBL_CT][rIspCamInfo.eIdx_ANR_TBL_ISO-1], // ANR settings for lower ISO
                             anr_tbl);  // Output
                   AAA_TRACE_END_G;
                   AAA_TRACE_END_NL;

               }
        }
        // Load it to ISP manager buffer.
        rNbc.put(anr);
        rNbc.putANR_TBL(anr_tbl);
    }


    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.anr2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);

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
        ISP_NVRAM_ANR2_T anr2 = m_IspNvramMgr.getANR2(rIdx.getIdx_ANR2());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sAnr2.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).ANR2;
                   m_ISP_INT.sAnr2.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).ANR2;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sAnr2.u2UpperIdx,
                            m_ISP_INT.sAnr2.u2LowerIdx);

                  AAA_TRACE_G(SmoothANR2);
                  AAA_TRACE_NL(SmoothANR2);

                  SmoothANR2(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2UpperIdx), // ANR2 settings for upper ISO
                            m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2LowerIdx), // ANR2 settings for lower ISO
                            anr2);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;


              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_ANR2(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2UpperIdx),
                                                  m_IspNvramMgr.getANR2(m_ISP_INT.sAnr2.u2LowerIdx), anr2);
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
prepareHw_PerFrame_CCR(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if ((bDisable) ||
        (!rNbc2.isCCTCCREnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_CCR_EN))) {
        rNbc2.setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rNbc2.setCCREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR(rIdx.getIdx_CCR());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sCcr.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).CCR;
                   m_ISP_INT.sCcr.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).CCR;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sCcr.u2UpperIdx,
                            m_ISP_INT.sCcr.u2LowerIdx);

                  AAA_TRACE_G(SmoothCCR);
                  AAA_TRACE_NL(SmoothCCR);

                  SmoothCCR(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getCCR(m_ISP_INT.sCcr.u2UpperIdx), // CCR settings for upper ISO
                            m_IspNvramMgr.getCCR(m_ISP_INT.sCcr.u2LowerIdx), // CCR settings for lower ISO
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

/*

MBOOL
Paramctrl::
prepareHw_PerFrame_BOK(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)

{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.bok.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_NBC2_T& rNbc2 = ISP_MGR_NBC2_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if ((bDisable) ||(!rNbc2.isCCTBOKEnable()) ||
        (rIspCamInfo.eIspProfile == EIspProfile_YUV_Reprocess))
    {
        rNbc2.setBOKEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rNbc2.setBOKEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BOK_T bok = m_IspNvramMgr.getBOK(rIdx.getIdx_BOK());

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BOK(rIspCamInfo, m_IspNvramMgr, bok);
        }

        // Load it to ISP manager buffer.
        rNbc2.put(bok);
    }

    return  MTRUE;
}

*/

MBOOL
Paramctrl::
prepareHw_PerFrame_PCA(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.pca.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_PCA_T& rPca = ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode);

    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.
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
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rPca.setEnable(MTRUE);
    }

    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

    if (m_ePCAMode == EPCAMode_360BIN) { // for VFB
#warning "VFB linker error"
#if 0
        CAM_LOGD_IF(m_bDebugEnable,"%s: loading vFB PCA (0x%x) ...", __FUNCTION__, halVFBTuning::getInstance().mHalVFBTuningGetPCA());
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(halVFBTuning::getInstance().mHalVFBTuningGetPCA());
#endif
    }
    else {
       if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl)
       {
           MBOOL fgNeedInter = MFALSE;
           MUINT32 u4CTlower, u4CTupper;

           EIndex_PCA_LUT_T eIdx_PCA_LUT = m_pIspTuningCustom->evaluate_SmoothPCA_LUT_idx(rIspCamInfo, fgNeedInter, u4CTlower, u4CTupper);

           //m_rIspCamInfo.eIdx_PCA_LUT = m_pIspTuningCustom->evaluate_SmoothPCA_LUT_idx(rIspCamInfo, fgNeedInter, u4CTlower, u4CTupper);

           m_pPcaMgr->setIdx(static_cast<MUINT32>(eIdx_PCA_LUT));

           if (fgNeedInter)
           {
               ISP_NVRAM_PCA_LUT_T* pca_lower;
               ISP_NVRAM_PCA_LUT_T* pca_upper;
               ISP_NVRAM_PCA_LUT_T pca_out;
               m_pPcaMgr->getLut( pca_lower, pca_upper);

               AAA_TRACE_G(SmoothPCA_TBL);
               AAA_TRACE_NL(SmoothPCA_TBL);

               SmoothPCA_TBL(rIspCamInfo.rAWBInfo.i4CCT, u4CTupper, u4CTlower,
                             *pca_upper, *pca_lower, pca_out, rIspCamInfo.i4LightValue_x10,
                             m_rIspParam.rPcaParam);
               AAA_TRACE_END_G;
               AAA_TRACE_END_NL;

               ISP_MGR_PCA_T::getInstance(m_eSensorDev).loadLut(reinterpret_cast<MUINT32*>(&pca_out.lut[0]));
           }
           else
           {
               m_pPcaMgr->loadLut();
           }

           m_pPcaMgr->loadConfig();

       }
       else
       {
        // (2) Invoke callback for customers to modify.
        if  (isDynamicTuning())
        {   // Dynamic Tuning: Enable
               m_pPcaMgr->setIdx(static_cast<MUINT32>(rIspCamInfo.eIdx_PCA_LUT));
        }

        // Check to see if it is needed to load LUT.
        switch  (getOperMode())
        {
        case EOperMode_Normal:
        case EOperMode_PureRaw:
            fgIsToLoadLut = m_pPcaMgr->isChanged();   // Load if changed.
            break;
        default:
            fgIsToLoadLut = MTRUE;                  // Force to load.
            break;
        }

        if (fgIsToLoadLut) {
            m_pPcaMgr->loadLut();
            m_pPcaMgr->loadConfig();
        }
    }
   }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_HFG(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.hfg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_HFG_T& rHfg = ISP_MGR_HFG_T::getInstance(m_eSensorDev);

    if ((bDisable) ||(!rHfg.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_HFG_EN)))
    {
        rHfg.setEnable(MFALSE);
    }
    else{
        rHfg.setEnable(MTRUE);
        // Get default NVRAM parameter
        ISP_NVRAM_HFG_T hfg = m_IspNvramMgr.getHFG(rIdx.getIdx_HFG());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sHfg.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).HFG;
                   m_ISP_INT.sHfg.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).HFG;


                  MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sHfg.u2UpperIdx,
                            m_ISP_INT.sHfg.u2LowerIdx);

                  AAA_TRACE_G(SmoothHFG);
                  AAA_TRACE_NL(SmoothHFG);

                  SmoothHFG(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getHFG(m_ISP_INT.sHfg.u2UpperIdx), // HFG settings for upper ISO
                            m_IspNvramMgr.getHFG(m_ISP_INT.sHfg.u2LowerIdx), // HFG settings for lower ISO
                            hfg);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;


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
prepareHw_PerFrame_EE(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ee.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_SEEE_T& rSeee = ISP_MGR_SEEE_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    if ((bDisable) || (!rSeee.isCCTEnable()) ||
        (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                   ||
        ((rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG)&&(!fgRPGEnable))||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_EE_EN)) )
    {
        rSeee.setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        rSeee.setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE(rIdx.getIdx_EE());

        //Interpolation ISP module
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
        {
              if ( rIspCamInfo.eIdx_ISO_L != rIspCamInfo.eIdx_ISO_U)
              {
                   m_ISP_INT.sEe.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_U, rIspCamInfo).EE;
                   m_ISP_INT.sEe.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO_L, rIspCamInfo).EE;


                  CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, rIspCamInfo.u4ISOValue,
                            m_ISP_INT.sEe.u2UpperIdx,
                            m_ISP_INT.sEe.u2LowerIdx);

                  AAA_TRACE_G(SmoothEE);
                  AAA_TRACE_NL(SmoothEE);

                  SmoothEE(rIspCamInfo.u4ISOValue,  // Real ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_U], // Upper ISO
                            m_rIspIsoEnv.IDX_Partition[rIspCamInfo.eIdx_ISO_L], // Lower ISO
                            m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2UpperIdx), // EE settings for upper ISO
                            m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2LowerIdx), // EE settings for lower ISO
                            ee);  // Output
                  AAA_TRACE_END_G;
                  AAA_TRACE_END_NL;


              }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            if (m_pIspTuningCustom->is_to_invoke_fine_tune(m_eSensorDev)){
                m_pIspTuningCustom->fine_tune_EE(rIspCamInfo, m_GmaExifInfo, m_LceExifInfo,
                                                  m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2UpperIdx),
                                                  m_IspNvramMgr.getEE(m_ISP_INT.sEe.u2LowerIdx), ee);
            }
            m_pIspTuningCustom->refine_EE(rIspCamInfo, m_IspNvramMgr, ee);

            if (rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
            {
                // User setting
                m_pIspTuningCustom->userSetting_EE(rIspCamInfo, rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);
            }
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
prepareHw_PerFrame_EFFECT(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.g2c.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_G2C_T& rG2c = ISP_MGR_G2C_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] effect(%d)", __FUNCTION__, rIspCamInfo.eIdx_Effect);

    if ((bDisable) || (!rG2c.isCCTEnable()) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_G2C_EN))
        //||(m_bMono)
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

/*

MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    ISP_NVRAM_NR3D_T nr3d = m_IspNvramMgr.getNR3D(rIdx.getIdx_NR3D());
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    //Interpolation ISP module
    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
    {
          const ISO_Partition_ENV_T* idx_env = static_cast<ISO_Partition_ENV_T*> (m_pIspTuningCustom->get_IDX_env_info (m_eSensorDev, rIspCamInfo.eIspProfile));
          EIndex_ISO_T eIDX_ISO_TOP = static_cast<EIndex_ISO_T> (idx_env->u4Length - 1);
          MUINT32 ISO_boundary = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO, rIspCamInfo.eIspProfile);

          if ( (rIspCamInfo.eIdx_ISO < static_cast<EIndex_ISO_T> (idx_env->u4Length - 1))
                && (rIspCamInfo.eIdx_ISO > eIDX_ISO_0))
          {
              if (rIspCamInfo.u4ISOValue >= ISO_boundary) {
                     m_ISP_INT.sNr3d.u4UpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1), rIspCamInfo.eIspProfile);
                     m_ISP_INT.sNr3d.u4LowerISO = ISO_boundary;
                     m_ISP_INT.sNr3d.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO+1, rIspCamInfo).NR3D;
                     m_ISP_INT.sNr3d.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO, rIspCamInfo).NR3D;
              }
              else {
                     m_ISP_INT.sNr3d.u4UpperISO = ISO_boundary;
                     m_ISP_INT.sNr3d.u4LowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1), rIspCamInfo.eIspProfile);
                     m_ISP_INT.sNr3d.u2UpperIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO, rIspCamInfo).NR3D;
                     m_ISP_INT.sNr3d.u2LowerIdx = getISPIndex_byISOIdx(rIspCamInfo.eIdx_ISO-1, rIspCamInfo).NR3D;
              }
              CAM_LOGD_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                        __FUNCTION__, rIspCamInfo.u4ISOValue,
                        m_ISP_INT.sNr3d.u4UpperISO,
                        m_ISP_INT.sNr3d.u4LowerISO,
                        m_ISP_INT.sNr3d.u2UpperIdx,
                        m_ISP_INT.sNr3d.u2LowerIdx);

              SmoothNR3D(rIspCamInfo.u4ISOValue,  // Real ISO
                        m_ISP_INT.sNr3d.u4UpperISO, // Upper ISO
                        m_ISP_INT.sNr3d.u4LowerISO, // Lower ISO
                        m_IspNvramMgr.getNR3D(m_ISP_INT.sNr3d.u2UpperIdx), // NR3D settings for upper ISO
                        m_IspNvramMgr.getNR3D(m_ISP_INT.sNr3d.u2LowerIdx), // NR3D settings for lower ISO
                        nr3d);  // Output

          }
    }


    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR3D(rIspCamInfo, m_IspNvramMgr, nr3d);
    }

    // Load it to ISP manager buffer
    ISP_MGR_NR3D_T::getInstance(m_eSensorDev).put(nr3d);

    return  MTRUE;
}

*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MFB(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    // Get default NVRAM parameter
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB(rIdx.getIdx_MFB());
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

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
prepareHw_PerFrame_MIXER3(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    ISP_NVRAM_MIXER3_T mixer3 = m_IspNvramMgr.getMIX3(rIdx.getIdx_MIX3());
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

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
prepareHw_PerFrame_LCE(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.lce.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
    ISP_MGR_LCE_T& rLce = ISP_MGR_LCE_T::getInstance(m_eSensorDev);

    if ((bDisable) ||(!rLce.isCCTEnable()) || (m_pLCSBuffer == NULL) ||
        (!(m_pFeatureCtrl[rIspCamInfo.eIspProfile] & M_LCE_EN)) ||
        (rIspCamInfo.bBypassLCE) ||
        (rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX))
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

        // Get Value from algo
        AAA_TRACE_G(calculateLCE);
        AAA_TRACE_NL(calculateLCE);
        m_pLceMgr->calculateLCE(&lce, &m_LceExifInfo, rIspCamInfo.rAEInfo, rIspCamInfo.eIspProfile,
                                rIspCamInfo.rLcs_INFO.lcs, pgn, ccm, (MUINT16*)m_pLCSBuffer->getImageBufferHeap()->getBufVA(0));
        AAA_TRACE_END_NL;
        AAA_TRACE_END_G;

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

