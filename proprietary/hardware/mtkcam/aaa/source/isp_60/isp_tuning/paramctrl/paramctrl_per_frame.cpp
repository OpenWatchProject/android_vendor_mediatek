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
//#include <awb_param.h>
#include <isp_tuning.h>
//#include <awb_param.h>
//#include <ae_param.h>
//#include <af_param.h>
//#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_config.h>
#include <isp_mgr_helper.h>
#include <ispfeature.h>
#include <isp_interpolation/isp_interpolation.h>
#include <ccm_mgr.h>
#include <ggm_mgr.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <dce_mgr.h>
#include <lsc/ILscMgr.h>
#include <mtkcam/drv/IHalSensor.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>
#include "isp_tuning_mgr.h"
#include <mtkcam/utils/sys/IFileCache.h>
#include <isp_config/isp_ae_config.h>
#include <isp_config/isp_awb_config.h>


#include <isp_config/isp_flicker_config.h> /* FLK config */


#include "private/PDTblGen.h"

//#include <hwutils/CameraProfile.h>
//#include "vfb_hal_base.h"
//#include <mtkcam/featureio/capturenr.h>

//define log control
#define EN_3A_FLOW_LOG        1
#define EN_3A_SCHEDULE_LOG    2

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam::TuningUtils;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_P1(RequestSet_T const RequestSet, MBOOL bReCalc, MINT32 i4SubsampleIdex)
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

    AAA_TRACE_ISP(applyToHwP1);

    m_pTuning->dequeBuffer(RequestTuningSet.vNumberSet.data());

    for (MINT32 i4InitNum = 0; i4InitNum < m_i4SubsampleCount; i4InitNum++)
    {
        fgRet = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev).apply_P1(0, m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_AF_STAT_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4InitNum)
            &&  ISP_MGR_PDO_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4Magic, i4InitNum)
            &&  ISP_MGR_FLK_CONFIG_T::getInstance(m_eSensorDev).apply(*m_pTuning, i4InitNum)
            &&  ISP_MGR_DGN_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_DGN::EDGN_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_WB_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_WB::EWB_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_BPC_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_BPC::EBPC_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
            // LCES should be after CCM(WB) & FLC
            &&  ISP_MGR_LCES_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_LCES::ELCES_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_LTM_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_LTM::ELTM_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
            &&  ISP_MGR_HLR_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_HLR::EHLR_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
            ;

        if(m_ERawPath != NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C){
            fgRet &= (ISP_MGR_OBC_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_OBC::EOBC_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      //Raw C doesn't support Direct YUV
                      && ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_SLK::ESLK_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      //DM should be after SLK_R1
                      && ISP_MGR_DM_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_DM::EDM_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      && ISP_MGR_FLC_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_FLC::EFLC_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      && ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_CCM::ECCM_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      && ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_GGM::EGGM_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      && ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_GGM::EGGM_R2, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      && ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_G2C::EG2C_R1, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      && ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_G2C::EG2C_R2, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      && ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_SLK::ESLK_R2, m_rIspCamInfo, *m_pTuning, i4InitNum)
                      //YNRS should be after SLK_R2
                      && ISP_MGR_YNRS_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_YNRS::EYNRS_R1, m_rIspCamInfo, *m_pTuning, i4InitNum));
        }
        else{
            //MOBC only exists on RawC
            fgRet &= ISP_MGR_MOBC_T::getInstance(m_eSensorDev).apply_P1(ISP_MGR_MOBC::EMOBC_R2, m_rIspCamInfo, *m_pTuning, i4InitNum);
        }

        //Debug Info
        ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo, *m_pTuning, i4InitNum);

        ISP_NVRAM_CAMCTL_T CTL_INFO ={0};
        ISP_MGR_CAMCTL_T::getInstance(m_eSensorDev).get(CTL_INFO);
        CAM_LOGD_IF(m_bDebugEnable, "EN:0x%x, EN2:0x%x, EN3:0x%x",
                    __FUNCTION__,
                    CTL_INFO.en.val,
                    CTL_INFO.en2.val,
                    CTL_INFO.en3.val
        );
    }


//Chooo WT
#if 0
    TUNING_MGR_TYPE_ENUM eType = TUNING_MGR_TYPE_SETTUNING;
    if(bReCalc) eType = TUNING_MGR_TYPE_CALLBACK;

    m_pTuning->enqueBuffer(eType);
#else
    m_pTuning->enqueBuffer();
#endif


    AAA_TRACE_END_ISP;

    if(m_pCcuIsp){
        struct ccu_ltm_perframe_data_in CCU_perFrameData = {0};
        CCU_perFrameData.magic_number = i4Magic;
        CCU_perFrameData.request_number = m_i4RequestNum;
        MUINT16 temp_idx = 0;
        temp_idx = getISPIndex(IDXCACHE_VALTYPE_CURRENT, m_rIspCamInfo, EModule_LTM, IDXCACHE_STAGE_P1);
        CCU_perFrameData.ltm_nvram_idx = (MUINT32)temp_idx;
        temp_idx = getISPIndex(IDXCACHE_VALTYPE_CURRENT, m_rIspCamInfo, EModule_HLR, IDXCACHE_STAGE_P1);
        CCU_perFrameData.hlr_nvram_idx = (MUINT32)temp_idx;

        CCU_perFrameData.hlr_cct = m_rIspCamInfo.rAWBInfo.i4CCT;

        CCU_perFrameData.ltm_en = ISP_MGR_LTM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_LTM::ELTM_R1);
        CCU_perFrameData.hlr_en = ISP_MGR_HLR_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_HLR::EHLR_R1);

        AAA_TRACE_CCU(CCUPERFRAMESETTING);
        if(m_pCcuIsp->getLtmInitStatus()){
            m_pCcuIsp->ccuControl(MSG_TO_CCU_LTM_PERFRAME_CTRL, &CCU_perFrameData, NULL);
        }
        AAA_TRACE_END_CCU;
    }
    else{
        CAM_LOGE("m_pCcuIsp is NULL");
    }

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
    AAA_TRACE_ISP(applyToHwP2);
    MBOOL fgRet = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] + pRegBuf(%p)", __FUNCTION__, pRegBuf);

    const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;
    dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(pRegBuf);

    if(rIspCamInfo.u1P2TuningUpdate != EPartKeep){
        ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));
    }

    MBOOL bLSC2En = ISP_MGR_LSC_T::getInstance(m_eSensorDev).isEnable(1);

    fgRet = MTRUE
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_OBC::EOBC_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_WB_T::getInstance(m_eSensorDev).apply_P2(    ISP_MGR_WB::EWB_D1,       rIspCamInfo, pReg)
        &&  ISP_MGR_DGN_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_DGN::EDGN_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_HLR_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_HLR::EHLR_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_LTM_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_LTM::ELTM_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev).apply_P2(   1,                        rIspCamInfo, pReg)
        &&  ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_SLK::ESLK_D1,     rIspCamInfo, bLSC2En, pReg)
        &&  ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_SLK::ESLK_D2,     rIspCamInfo, bLSC2En, pReg)
        &&  ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_SLK::ESLK_D3,     rIspCamInfo, bLSC2En, pReg)
        &&  ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_SLK::ESLK_D4,     rIspCamInfo, bLSC2En, pReg)
        &&  ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_SLK::ESLK_D5,     rIspCamInfo, bLSC2En, pReg)
        &&  ISP_MGR_SLK_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_SLK::ESLK_D6,     rIspCamInfo, bLSC2En, pReg)
        &&  ISP_MGR_BPC_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_BPC::EBPC_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_DM_T::getInstance(m_eSensorDev).apply_P2(    ISP_MGR_DM::EDM_D1,       rIspCamInfo, pReg)
        &&  ISP_MGR_LDNR_T::getInstance(m_eSensorDev).apply_P2(  ISP_MGR_LDNR::ELDNR_D1,   rIspCamInfo, pReg)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_GGM::EGGM_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_G2CX_T::getInstance(m_eSensorDev).apply_P2(  ISP_MGR_G2CX::EG2CX_D1,   rIspCamInfo, pReg)
        &&  ISP_MGR_C2G_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_C2G::EC2G_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_IGGM_T::getInstance(m_eSensorDev).apply_P2(  ISP_MGR_IGGM::EIGGM_D1,   rIspCamInfo, pReg)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_GGM::EGGM_D3,     rIspCamInfo, pReg)
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_G2C::EG2C_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_CCM::ECCM_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_CCM::ECCM_D3,     rIspCamInfo, pReg)
        &&  ISP_MGR_EE_T::getInstance(m_eSensorDev).apply_P2(    ISP_MGR_EE::EEE_D1,       rIspCamInfo, pReg)
        &&  ISP_MGR_CNR_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_CNR::ECNR_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_DCES_T::getInstance(m_eSensorDev).apply_P2(  ISP_MGR_DCES::EDCES_D1,   rIspCamInfo, pReg)
        &&  ISP_MGR_DCE_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_DCE::EDCE_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_FLC_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_FLC::EFLC_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_LCE_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_LCE::ELCE_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_COLOR_T::getInstance(m_eSensorDev).apply_P2( ISP_MGR_COLOR::ECOLOR_D1, rIspCamInfo, pReg)
        &&  ISP_MGR_NDG_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_NDG::ENDG_D1,     rIspCamInfo, pReg)
        &&  ISP_MGR_NDG_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_NDG::ENDG_D2,     rIspCamInfo, pReg)
        &&  ISP_MGR_MIX_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_MIX::EMIX_D3,     rIspCamInfo, pReg)
        &&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply_P2(  ISP_MGR_NR3D::ENR3D_D1,   rIspCamInfo, pReg)
        &&  ISP_MGR_YNR_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_YNR::EYNR_D1,     rIspCamInfo, pReg)
        ;

    if(rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview ||
       rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW){
        fgRet &=(  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_GGM::EGGM_D2,     rIspCamInfo, pReg)
                 &&ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply_P2(   ISP_MGR_CCM::ECCM_D2,     rIspCamInfo, pReg));
    }
    CAM_LOGD_IF(m_bDebugEnable, "[%s] - Before Constraint FrmId(%d), Profile(%d), YUV:0x%x, YUV2:0x%x, RGB:0x%x, RGB2:0x%x, DMA:0x%x, DMA2:0x%x ",
        __FUNCTION__,
        rIspCamInfo.u4Id,
        rIspCamInfo.rMapping_Info.eIspProfile,
        pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Raw,
        pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Raw,
        pReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Raw,
        pReg->DIPCTL_D1A_DIPCTL_RGB_EN2.Raw,
        pReg->DIPCTL_D1A_DIPCTL_DMA_EN1.Raw,
        pReg->DIPCTL_D1A_DIPCTL_DMA_EN2.Raw
        );

    //factory no need BPCI
    if(m_eOperMode == EOperMpde_Factory){
        pReg->BPC_D1A_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN = 0;
        pReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Bits.DIPCTL_LSC_D1_EN = 0;
        pReg->DIPCTL_D1A_DIPCTL_DMA_EN1.Bits.DIPCTL_IMGCI_D1_EN = 0;
        pReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Bits.DIPCTL_LTM_D1_EN = 0;
        pReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Bits.DIPCTL_HLR_D1_EN = 0;
        pReg->YNR_D1A_YNR_CON1.Bits.YNR_LCE_LINK = 0;
        pReg->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK = 0;
    }

    // the protection of HW limitations
    ispP2HWConstraintSet(pReg);   // the protection of HW limitations

    CAM_LOGD_IF(m_bDebugEnable, "[%s] - After Constraint FrmId(%d), Profile(%d), YUV:0x%x, YUV2:0x%x, RGB:0x%x, RGB2:0x%x, DMA:0x%x, DMA2:0x%x ",
        __FUNCTION__,
        rIspCamInfo.u4Id,
        rIspCamInfo.rMapping_Info.eIspProfile,
        pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Raw,
        pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Raw,
        pReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Raw,
        pReg->DIPCTL_D1A_DIPCTL_RGB_EN2.Raw,
        pReg->DIPCTL_D1A_DIPCTL_DMA_EN1.Raw,
        pReg->DIPCTL_D1A_DIPCTL_DMA_EN2.Raw
        );

    CAM_LOGD_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);
    AAA_TRACE_END_ISP;

    return fgRet;
}

MBOOL
Paramctrl::
applyToHw_PerFrame_MFB(MINT32 /*flowType*/, const ISP_INFO_T& rIspInfo, void* pMfbBuf)
{

    AAA_TRACE_ISP(applyToHwMFB);

    MBOOL fgRet = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] + pMfbBuf(%p)", __FUNCTION__, pMfbBuf);

    const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;
    mfb_reg_t* pMfb = reinterpret_cast<mfb_reg_t*>(pMfbBuf);
    ::memset((void*)pMfb, 0, sizeof(mfb_reg_t));

    fgRet = MTRUE
        &&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply_MFB(ISP_MGR_MFB::EMFB_D1, rIspCamInfo, pMfb)
        ;

    CAM_LOGD_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);


    AAA_TRACE_END_ISP;

    return fgRet;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_P1()
{
    MBOOL fgRet = MTRUE;

    AAA_TRACE_ISP(3APREPARE);

    fgRet &= (prepareHw_PerFrame_AF(m_rIspCamInfo.u4Id)
              &&  prepareHw_PerFrame_PDO(m_rIspCamInfo.u4Id)
              &&  prepareHw_PerFrame_AE(m_rIspCamInfo.u4Id)
              &&  prepareHw_PerFrame_AWB()
              &&  prepareHw_PerFrame_Flicker(m_rIspCamInfo.u4Id)
              );

    AAA_TRACE_END_ISP;

    if  ( ! fgRet )
    {
        CAM_LOGE("prepareHw 3A Part error");
        goto lbExit;
    }

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagStart); // Profiling Start.
    decision_PerFrame_P1_OBC();
    decision_PerFrame_P1_BPC();
    decision_PerFrame_P1_CT();
    decision_PerFrame_P1_PDC();
    decision_PerFrame_P1_DGN();
    decision_PerFrame_P1_LSC();
    decision_PerFrame_P1_WB();
    decision_PerFrame_P1_LTM();
    //HLR should be after HLR
    decision_PerFrame_P1_HLR();
    decision_PerFrame_P1_LCES();
    //Direct YUV
    decision_PerFrame_P1_SLK(); //must after LSC
    decision_PerFrame_P1_DM();
    decision_PerFrame_P1_FLC();
    decision_PerFrame_P1_CCM();
    decision_PerFrame_P1_GGM();
    decision_PerFrame_P1_G2C();
    decision_PerFrame_P1_YNRS();

    AAA_TRACE_ISP(GETP1INDEX);

    get_P1_AllIndex(m_rIspCamInfo);

    AAA_TRACE_END_ISP;


    AAA_TRACE_ALG(GETP1ALGORESULT);
    fgRet &= (getNvram_PerFrame_OBC(       ISP_MGR_OBC::EOBC_R1,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_BPC(    ISP_MGR_BPC::EBPC_R1,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_CT(     ISP_MGR_BPC::EBPC_R1,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_PDC(    ISP_MGR_BPC::EBPC_R1,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              //Direct YUV
              && getNvram_PerFrame_DM(     ISP_MGR_DM::EDM_R1,     m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_FLC(    ISP_MGR_FLC::EFLC_R1,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_CCM_P1( ISP_MGR_CCM::ECCM_R1,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              //LCES must be after CCM
              && getNvram_PerFrame_LCES(   ISP_MGR_LCES::ELCES_R1, m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_GGM(    ISP_MGR_GGM::EGGM_R1,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_GGM(    ISP_MGR_GGM::EGGM_R2,   m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_YNRS(   ISP_MGR_YNRS::EYNRS_R1, m_rIspCamInfo, IDXCACHE_STAGE_P1)
              && getNvram_PerFrame_YNR_TBL(ISP_MGR_YNRS::EYNRS_R1, m_rIspCamInfo, IDXCACHE_STAGE_P1)
              );
    AAA_TRACE_END_ALG;

    if  ( ! fgRet )
    {
        CAM_LOGE("prepareHw P1_ISP Part error");
        goto lbExit;
    }

    // wait TSF
    fgRet &= prepareHw_PerFrame_LSC(0) ;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

lbExit:
    return  fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_P2(const ISP_INFO_T& rIspInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] +", __FUNCTION__);

    decision_PerFrame_P2_OBC(rIspInfo.rCamInfo);
    decision_PerFrame_P2_BPC(rIspInfo.rCamInfo);
    decision_PerFrame_P2_CT(rIspInfo.rCamInfo);
    decision_PerFrame_P2_PDC(rIspInfo.rCamInfo);
    decision_PerFrame_P2_DGN(rIspInfo.rCamInfo);
    decision_PerFrame_P2_LSC(rIspInfo.rCamInfo);
    decision_PerFrame_P2_WB(rIspInfo.rCamInfo);
    decision_PerFrame_P2_LTM(rIspInfo.rCamInfo);
    //HLR must be after LTM
    decision_PerFrame_P2_HLR(rIspInfo.rCamInfo);
//ISP Pass2 After Raw
    //SLK must after LSC
    decision_PerFrame_P2_SLK(rIspInfo.rCamInfo);
    decision_PerFrame_P2_DM(rIspInfo.rCamInfo);
    decision_PerFrame_P2_LDNR(rIspInfo.rCamInfo);
    decision_PerFrame_P2_FLC(rIspInfo.rCamInfo);
    decision_PerFrame_P2_CCM(rIspInfo.rCamInfo);
    decision_PerFrame_P2_GGM(rIspInfo.rCamInfo);
    decision_PerFrame_P2_G2C(rIspInfo.rCamInfo);
    decision_PerFrame_P2_YNR(rIspInfo.rCamInfo);
    decision_PerFrame_P2_YNR_LINK(rIspInfo.rCamInfo);
    decision_PerFrame_P2_LCE(rIspInfo.rCamInfo);
    decision_PerFrame_P2_DCES(rIspInfo.rCamInfo);
    decision_PerFrame_P2_DCE(rIspInfo.rCamInfo);
    decision_PerFrame_P2_EE(rIspInfo.rCamInfo);
    decision_PerFrame_P2_CNR(rIspInfo.rCamInfo);
    decision_PerFrame_P2_CCR(rIspInfo.rCamInfo);
    decision_PerFrame_P2_ABF(rIspInfo.rCamInfo);
    decision_PerFrame_P2_BOK(rIspInfo.rCamInfo);
    // NDG must be after YNR & CNR
    decision_PerFrame_P2_NDG(rIspInfo.rCamInfo);
    decision_PerFrame_P2_COLOR(rIspInfo.rCamInfo);
//Feature
    decision_PerFrame_P2_NR3D(rIspInfo.rCamInfo);
    decision_PerFrame_P2_MIX(rIspInfo.rCamInfo);
//Normal Pass2 Flow LPCNR always be off
    decision_PerFrame_P2_LPCNR(rIspInfo.rCamInfo);

    MBOOL  b1Update = MFALSE;
    ISOThresholdCheck(rIspInfo, b1Update);

    AAA_TRACE_ISP(GETP2INDEX);

    get_P2_AllIndex(rIspInfo.rCamInfo);

    AAA_TRACE_END_ISP;


    AAA_TRACE_ALG(GETP2ALGORESULT);

    MBOOL fgRet = MTRUE
        && getNvram_PerFrame_OBC(     ISP_MGR_OBC::EOBC_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_BPC(     ISP_MGR_BPC::EBPC_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_CT(      ISP_MGR_BPC::EBPC_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_PDC(     ISP_MGR_BPC::EBPC_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_DM(      ISP_MGR_DM::EDM_D1,       rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_LDNR(    ISP_MGR_LDNR::ELDNR_D1,   rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_CCM_P2(  ISP_MGR_CCM::ECCM_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_CCM_P2(  ISP_MGR_CCM::ECCM_D2,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        //GGM_D3 include GGM_D1, GGM_D3, IGGM_D1
        && getNvram_PerFrame_GGM(     ISP_MGR_GGM::EGGM_D3,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_GGM(     ISP_MGR_GGM::EGGM_D2,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_LCE(     ISP_MGR_LCE::ELCE_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_G2CX(    ISP_MGR_G2CX::EG2CX_D1,   rIspInfo.rCamInfo)
        //YNR must be after GGM & LCE
        && getNvram_PerFrame_YNR(     ISP_MGR_YNR::EYNR_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_YNR_TBL( ISP_MGR_YNR::EYNR_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_YNR_FACE(ISP_MGR_YNR::EYNR_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_DCES(    ISP_MGR_DCES::EDCES_D1,   rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        // DCE must be after LCE
        && getNvram_PerFrame_DCE(     ISP_MGR_DCE::EDCE_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        // FLC must be after DCE
        && getNvram_PerFrame_FLC(     ISP_MGR_FLC::EFLC_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_EE(      ISP_MGR_EE::EEE_D1,       rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_CNR(     ISP_MGR_CNR::ECNR_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_CCR(     ISP_MGR_CNR::ECNR_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_ABF(     ISP_MGR_CNR::ECNR_D1,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_COLOR(   ISP_MGR_COLOR::ECOLOR_D1, rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_MIX(     ISP_MGR_MIX::EMIX_D3,     rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && getNvram_PerFrame_NR3D(    ISP_MGR_NR3D::ENR3D_D1,   rIspInfo.rCamInfo, IDXCACHE_STAGE_P2)
        && prepareHw_PerFrame_LSC_2(1, rIspInfo)
        ;

        AAA_TRACE_END_ALG;

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

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_LSC(0) //LSC pass1
        //&&  prepareHw_PerFrame_WB()
            ;


    //Exception of dynamic CCM
    if(isDynamicCCM())
        fgRet &= getNvram_PerFrame_CCM_P1(ISP_MGR_CCM::ECCM_R1, m_rIspCamInfo, IDXCACHE_STAGE_P1);


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
    //Chooo
    //prepareHw_PerFrame_DM(rIspInfo.rCamInfo);
    //prepareHw_PerFrame_PGN(rIspInfo.rCamInfo);
    prepareHw_PerFrame_LSC_2(1, rIspInfo); //lsc pass2

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Paramctrl::
ispP2HWConstraintSet(dip_x_reg_t* pReg)
{
    // Bokeh HW issue
    if(pReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_MODE){

        pReg->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK = 0;
        pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_SLK_D3_EN = 1;
        pReg->SLK_D3A_SLK_RZ.Bits.SLK_HRZ_COMP = 2048;
        pReg->SLK_D3A_SLK_RZ.Bits.SLK_VRZ_COMP = 2048;
    }

    if(pReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_VIDEO_MODE){
		if (pReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_CNR_SCALE_MODE == 3)
            pReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_CNR_SCALE_MODE =2;
        pReg->CNR_D1A_CNR_ABF_CON1.Bits.CNR_ABF_NSR_IDX = 0;
        pReg->CNR_D1A_CNR_CNR_CON2.Bits.CNR_CNR_FLT_C = 0;
    }

    //SLK Part
    MBOOL SLK_D1_EN  = pReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Bits.DIPCTL_SLK_D1_EN;
    MBOOL SLK_D2_EN  = pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_SLK_D2_EN;
    MBOOL SLK_D3_EN  = pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_SLK_D3_EN;
    MBOOL SLK_D4_EN  = pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_SLK_D4_EN;
    MBOOL SLK_D5_EN  = pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_SLK_D5_EN;
    MBOOL SLK_D6_EN  = pReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Bits.DIPCTL_SLK_D6_EN;

    //DM_D1
    if(!SLK_D1_EN)
        pReg->DM_D1A_DM_SL_CTL.Bits.DM_SL_EN = 0;

    //YNR_D1
    if(!SLK_D2_EN)
        pReg->YNR_D1A_YNR_CON1.Bits.YNR_SL2_LINK = 0;

    //CNR_D1
    if(!SLK_D3_EN){
        pReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_CNR_SL2_LINK = 0;
        pReg->CNR_D1A_CNR_CNR_MED11.Bits.CNR_BPC_SL2_LINK = 0;
        pReg->CNR_D1A_CNR_CCR_CON.Bits.CNR_CCR_SL2_LINK = 0;
    }

    //EE_D1
    if(!SLK_D4_EN){
        pReg->EE_D1A_EE_LUMA_SLNK_CTRL.Bits.EE_GLUT_LINK_EN = 0;
        pReg->EE_D1A_EE_CE_SL_CTRL.Bits.EE_CE_SLMOD_EN = 0;
    }

    //NR3D_D1
    if(!SLK_D5_EN && pReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_NR3D_D1_EN)
        pReg->NR3D_D1A_NR3D_NR3D_ON_CON.Bits.NR3D_sl2_off = 1;

    //LDNR_D1
    if(!SLK_D6_EN)
        pReg->LDNR_D1A_LDNR_SL.Bits.LDNR_SL_EN = 0;

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).getDMGItable() == NULL){
       pReg->BPC_D1A_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN = 0;
    }

    if(pReg->BPC_D1A_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN == 0){
        pReg->BPC_D1A_BPC_PDC_CON.Bits.BPC_PDC_EN = 0;
    }

    //YNR_LCE issue
    MBOOL bLCE_D1_EN = pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_LCE_D1_EN;
    pReg->DIPCTL_D1A_DIPCTL_MISC_SEL.Bits.DIPCTL_YNR_GMAP_LTM_MODE = bLCE_D1_EN;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Paramctrl::
ispP2IdentitySet(dip_x_reg_t* pReg, MBOOL bReset)
{
    if(pReg != nullptr){
		if (bReset)
            ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));
    }
    else{
        CAM_LOGD("Error: pReg is nullptr!");
        return;
    }

    CAM_LOGD("ISP P2 Identity Setting");

    pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_C2G_D1_EN = 1;
    pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_IGGM_D1_EN = 1;
    pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_GGM_D3_EN = 1;
    pReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_G2C_D1_EN = 1;

    //Dummy Bit to Notify P2 Driver
    pReg->DIPCTL_D1A_DIPCTL_RGB_EN2.Bits.DIPCTL_SW_P2_IDENDITY_EN = 1;

    pReg->C2G_D1A_C2G_CONV_0A.Bits.C2G_CNV_00 = 512;
    pReg->C2G_D1A_C2G_CONV_0A.Bits.C2G_CNV_01 = 0;

    pReg->C2G_D1A_C2G_CONV_0B.Bits.C2G_CNV_02 = 0;
    pReg->C2G_D1A_C2G_CONV_0B.Bits.C2G_Y_OFST = 0;

    pReg->C2G_D1A_C2G_CONV_1A.Bits.C2G_CNV_10 = 0;
    pReg->C2G_D1A_C2G_CONV_1A.Bits.C2G_CNV_11 = 512;

    pReg->C2G_D1A_C2G_CONV_1B.Bits.C2G_CNV_12 = 0;
    pReg->C2G_D1A_C2G_CONV_1B.Bits.C2G_U_OFST = 511;

    pReg->C2G_D1A_C2G_CONV_2A.Bits.C2G_CNV_20 = 0;
    pReg->C2G_D1A_C2G_CONV_2A.Bits.C2G_CNV_21 = 0;

    pReg->C2G_D1A_C2G_CONV_2B.Bits.C2G_CNV_22 = 512;
    pReg->C2G_D1A_C2G_CONV_2B.Bits.C2G_V_OFST = 511;


    pReg->IGGM_D1A_IGGM_CTRL.Bits.IGGM_LNR = 1;

    pReg->GGM_D3A_GGM_CTRL.Bits.GGM_LNR = 1;

    pReg->G2C_D1A_G2C_CONV_0A.Bits.G2C_CNV_00 = 512;
    pReg->G2C_D1A_G2C_CONV_0A.Bits.G2C_CNV_01 = 0;

    pReg->G2C_D1A_G2C_CONV_0B.Bits.G2C_CNV_02 = 0;
    pReg->G2C_D1A_G2C_CONV_0B.Bits.G2C_Y_OFST = 0;

    pReg->G2C_D1A_G2C_CONV_1A.Bits.G2C_CNV_10 = 0;
    pReg->G2C_D1A_G2C_CONV_1A.Bits.G2C_CNV_11 = 512;

    pReg->G2C_D1A_G2C_CONV_1B.Bits.G2C_CNV_12 = 0;
    pReg->G2C_D1A_G2C_CONV_1B.Bits.G2C_U_OFST = -511;

    pReg->G2C_D1A_G2C_CONV_2A.Bits.G2C_CNV_20 = 0;
    pReg->G2C_D1A_G2C_CONV_2A.Bits.G2C_CNV_21 = 0;

    pReg->G2C_D1A_G2C_CONV_2B.Bits.G2C_CNV_22 = 512;
    pReg->G2C_D1A_G2C_CONV_2B.Bits.G2C_V_OFST = -511;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_AF(MUINT32 MagicNum, AFResultConfig_T* pReconfigAF)
{
    AFResultConfig_T *m_pAFResultConfig = NULL;
    m_pAFResultConfig = (AFResultConfig_T*)m_pResultPoolObj->getResult(MagicNum, E_AF_CONFIGRESULTTOISP, __FUNCTION__);

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get(#%d) AF config result to ResutlPool before %p", __FUNCTION__, MagicNum, m_pAFResultConfig);

    if(m_pAFResultConfig == NULL)
    {
       MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
       m_pResultPoolObj->getHistory(rHistoryReqMagic);
       CAM_LOGD_IF(m_bDebugEnable,"[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d)", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);

       CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req1) = (#%d)", __FUNCTION__, rHistoryReqMagic[1]);
       m_pAFResultConfig = (AFResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[1], E_AF_CONFIGRESULTTOISP, __FUNCTION__);

       if(m_pAFResultConfig == NULL)
       {
           CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req0) = (#%d)", __FUNCTION__, rHistoryReqMagic[0]);
           m_pAFResultConfig = (AFResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[0], E_AF_CONFIGRESULTTOISP, __FUNCTION__);
       }
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get AF config result to ResutlPool after %p", __FUNCTION__, m_pAFResultConfig);

    if(m_pAFResultConfig)
    {
        if (pReconfigAF)
        {
            pReconfigAF = m_pAFResultConfig;
        }
        else
        {
            CAM_LOGD_IF(m_bDebugEnable,"HW-%s : enableAFHw(%d)", __FUNCTION__, m_pAFResultConfig->afIspRegInfo.enableAFHw);
            ISP_MGR_AF_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_pAFResultConfig->afIspRegInfo));
        }
    }

    return MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_PDO(MUINT32 MagicNum, isp_pdo_cfg_t* pReconfigPDO)
{
    isp_pdo_cfg_t *m_pPDOResultConfig = NULL;
    m_pPDOResultConfig = (isp_pdo_cfg_t*)m_pResultPoolObj->getResult(MagicNum, E_PDO_CONFIGRESULTTOISP, __FUNCTION__);

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get(#%d) PDO config result to ResutlPool before %p", __FUNCTION__, MagicNum, m_pPDOResultConfig);

    if(m_pPDOResultConfig == NULL)
    {
       MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
       m_pResultPoolObj->getHistory(rHistoryReqMagic);
       CAM_LOGD_IF(m_bDebugEnable,"[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d)", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);

       CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req1) = (#%d)", __FUNCTION__, rHistoryReqMagic[1]);
       m_pPDOResultConfig = (isp_pdo_cfg_t*)m_pResultPoolObj->getResult(rHistoryReqMagic[1], E_PDO_CONFIGRESULTTOISP, __FUNCTION__);

       if(m_pPDOResultConfig == NULL)
       {
           CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req0) = (#%d)", __FUNCTION__, rHistoryReqMagic[0]);
           m_pPDOResultConfig = (isp_pdo_cfg_t*)m_pResultPoolObj->getResult(rHistoryReqMagic[0], E_PDO_CONFIGRESULTTOISP, __FUNCTION__);
       }
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get PDO config result to ResutlPool after %p", __FUNCTION__, m_pPDOResultConfig);

    if(m_pPDOResultConfig)
    {
        if (pReconfigPDO)
        {
            pReconfigPDO = m_pPDOResultConfig;
        }
        else
        {
            ISP_MGR_PDO_CONFIG_T::getInstance(m_eSensorDev).configReg(m_pPDOResultConfig);
        }
    }

    return MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_AE(MUINT32 MagicNum, AEResultConfig_T* pWT_ReConfigAE)
{
    AEResultConfig_T *m_pAEResultConfig = NULL;
    m_pAEResultConfig = (AEResultConfig_T*)m_pResultPoolObj->getResult(MagicNum, E_AE_CONFIGRESULTTOISP, __FUNCTION__);

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get(#%d) AE config result to ResutlPool before %p", __FUNCTION__, MagicNum, m_pAEResultConfig);

    if(m_pAEResultConfig == NULL)
    {
       MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
       m_pResultPoolObj->getHistory(rHistoryReqMagic);
       CAM_LOGD_IF(m_bDebugEnable,"[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d)", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);

       CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req1) = (#%d)", __FUNCTION__, rHistoryReqMagic[1]);
       m_pAEResultConfig = (AEResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[1], E_AE_CONFIGRESULTTOISP, __FUNCTION__);

       if(m_pAEResultConfig == NULL)
       {
           CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req0) = (#%d)", __FUNCTION__, rHistoryReqMagic[0]);
           m_pAEResultConfig = (AEResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[0], E_AE_CONFIGRESULTTOISP, __FUNCTION__);
       }
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get AE config result to ResutlPool after %p", __FUNCTION__, m_pAEResultConfig);
    if(m_pAEResultConfig)
    {
       if(pWT_ReConfigAE)
       {
          pWT_ReConfigAE = m_pAEResultConfig;
       }else
       {
          ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_pAEResultConfig->rAERegInfo));
       }
    }
    return MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_AWB()
{
    AWBResultConfig_T *m_pAWBResultConfig = NULL;
    m_pAWBResultConfig = (AWBResultConfig_T*)m_pResultPoolObj->getResult(m_rIspCamInfo.u4Id, E_AWB_CONFIGRESULTTOISP, __FUNCTION__);

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get(#%d) AWB config result to ResutlPool before %p", __FUNCTION__, m_rIspCamInfo.u4Id, m_pAWBResultConfig);

    if(m_pAWBResultConfig == NULL)
    {
       MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
       m_pResultPoolObj->getHistory(rHistoryReqMagic);
       CAM_LOGD_IF(m_bDebugEnable,"[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d)", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);

       CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req1) = (#%d)", __FUNCTION__, rHistoryReqMagic[1]);
       m_pAWBResultConfig = (AWBResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[1], E_AWB_CONFIGRESULTTOISP, __FUNCTION__);

       if(m_pAWBResultConfig == NULL)
       {
           CAM_LOGW("[%s] get History (Req0) = (#%d)", __FUNCTION__, rHistoryReqMagic[0]);
           m_pAWBResultConfig = (AWBResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[0], E_AWB_CONFIGRESULTTOISP, __FUNCTION__);
       }
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get AWB config result to ResutlPool after %p", __FUNCTION__, m_pAWBResultConfig);

    ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).configReg(&(m_pAWBResultConfig->rAWBRegInfo));

    return MTRUE;

}

MBOOL
Paramctrl::
prepareHw_PerFrame_Flicker(MUINT32 MagicNum, FLKResultConfig_T* pWT_ReConfigFLK)
{
    FLKResultConfig_T *m_pFLKResultConfig = NULL;
    m_pFLKResultConfig = (FLKResultConfig_T*)m_pResultPoolObj->getResult(MagicNum, E_FLK_CONFIGRESULTTOISP, __FUNCTION__);

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get(#%d) FLK config result to ResutlPool before %p", __FUNCTION__, MagicNum, m_pFLKResultConfig);

    if(m_pFLKResultConfig == NULL)
    {
       MINT32 rHistoryReqMagic[HistorySize] = {0,0,0};
       m_pResultPoolObj->getHistory(rHistoryReqMagic);
       CAM_LOGD_IF(m_bDebugEnable,"[%s] History (Req0, Req1, Req2) = (#%d, #%d, #%d)", __FUNCTION__, rHistoryReqMagic[0], rHistoryReqMagic[1], rHistoryReqMagic[2]);

       CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req1) = (#%d)", __FUNCTION__, rHistoryReqMagic[1]);
       m_pFLKResultConfig = (FLKResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[1], E_FLK_CONFIGRESULTTOISP, __FUNCTION__);

       if(m_pFLKResultConfig == NULL)
       {
           CAM_LOGD_IF(m_bDebugEnable,"[%s] get History (Req0) = (#%d)", __FUNCTION__, rHistoryReqMagic[0]);
           m_pFLKResultConfig = (FLKResultConfig_T*)m_pResultPoolObj->getResult(rHistoryReqMagic[0], E_FLK_CONFIGRESULTTOISP, __FUNCTION__);
       }
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s] get FLK config result to ResutlPool after %p", __FUNCTION__, m_pFLKResultConfig);
    if(m_pFLKResultConfig)
    {
        if(pWT_ReConfigFLK)
        {
            pWT_ReConfigFLK = m_pFLKResultConfig;
        }else{
            ISP_MGR_FLK_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_eSensorDev)).configReg(m_pFLKResultConfig);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_OBC()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_OBC_EN) ? MTRUE : MFALSE;

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;

    //OBC R1
    ISP_MGR_OBC_T& rObc = ISP_MGR_OBC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bOBC_R1
        || (!rObc.isCCTEnable())
        || (!bProfileRequest)
        ||  bRawC)
    {
        rObc.setEnable(ISP_MGR_OBC::EOBC_R1, MFALSE);
    }
    else
    {
        rObc.setEnable(ISP_MGR_OBC::EOBC_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_OBC_R1(%d), CCTEnable(%d), ProfileRequest(%d), RawC(%d)",
        __FUNCTION__
        , rObc.isEnable(ISP_MGR_OBC::EOBC_R1)
        , m_DebugCTL_Disable.bOBC_R1
        , rObc.isCCTEnable()
        , bProfileRequest
        , bRawC);

    //MOBC R2
    ISP_MGR_MOBC_T& rMobc = ISP_MGR_MOBC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bMOBC_R2
        || (!rMobc.isCCTEnable())
        || (!bProfileRequest)
        || (!bRawC))
    {
        rMobc.setEnable(ISP_MGR_MOBC::EMOBC_R2, MFALSE);
    }
    else
    {
        rMobc.setEnable(ISP_MGR_MOBC::EMOBC_R2, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_MOBC_R2(%d), CCTEnable(%d), ProfileRequest(%d), RawC(%d)",
        __FUNCTION__
        , rMobc.isEnable(ISP_MGR_MOBC::EMOBC_R2)
        , m_DebugCTL_Disable.bMOBC_R2
        , rMobc.isCCTEnable()
        , bProfileRequest
        , bRawC);


    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_BPC()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_BPC_EN) ? MTRUE : MFALSE;

    //BPC_BPC_R1
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bBPC_BPC_R1
        || (!rBpc.isCCTBPCEnable())
        || (!bProfileRequest))
    {
        rBpc.setBPCEnable(ISP_MGR_BPC::EBPC_R1, MFALSE);
    }
    else
    {
        rBpc.setBPCEnable(ISP_MGR_BPC::EBPC_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_BPC_R1(%d), CCTBPCEnable(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rBpc.isBPCEnable(ISP_MGR_BPC::EBPC_R1)
        , m_DebugCTL_Disable.bBPC_BPC_R1
        , rBpc.isCCTBPCEnable()
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_CT()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_CT_EN) ? MTRUE : MFALSE;

    //BPC_CT_R1
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bBPC_CT_R1
        || (!rBpc.isCCTCTEnable())
        || (!bProfileRequest))
    {
        rBpc.setCTEnable(ISP_MGR_BPC::EBPC_R1, MFALSE);
    }
    else
    {
        rBpc.setCTEnable(ISP_MGR_BPC::EBPC_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_CT_R1(%d), CCTCTEnable(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rBpc.isCTEnable(ISP_MGR_BPC::EBPC_R1)
        , m_DebugCTL_Disable.bBPC_CT_R1
        , rBpc.isCCTCTEnable()
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_PDC()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_PDC_EN) ? MTRUE : MFALSE;

    //BPC_PDC_R1
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bBPC_PDC_R1
        || (!rBpc.isCCTPDCEnable())
        || (!bProfileRequest))
    {
        rBpc.setPDCEnable(ISP_MGR_BPC::EBPC_R1, MFALSE);
    }
    else
    {
        rBpc.setPDCEnable(ISP_MGR_BPC::EBPC_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_PDC_R1(%d), CCTPDCEnable(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rBpc.isPDCEnable(ISP_MGR_BPC::EBPC_R1)
        , m_DebugCTL_Disable.bBPC_PDC_R1
        , rBpc.isCCTPDCEnable()
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_DGN()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_DGN_EN) ? MTRUE : MFALSE;

    //DGN R1
    ISP_MGR_DGN_T& rDgn = ISP_MGR_DGN_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bDGN_R1
        || (!bProfileRequest))
    {
        rDgn.setEnable(ISP_MGR_DGN::EDGN_R1, MFALSE);
    }
    else
    {
        rDgn.setEnable(ISP_MGR_DGN::EDGN_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_DGN_R1(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rDgn.isEnable(ISP_MGR_DGN::EDGN_R1)
        , m_DebugCTL_Disable.bDGN_R1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_LSC()
{
    MBOOL bProfileRequest = MTRUE;

    const LSCConfigResult_T* r_pISPLscResult = (LSCConfigResult_T*)m_pResultPoolObj->getResult(m_rIspCamInfo.u4Id, E_LSC_CONFIGRESULTTOISP, __FUNCTION__);

    if(r_pISPLscResult!=NULL)
    {
        memcpy(&m_pISPLscResult, r_pISPLscResult, sizeof(LSCConfigResult_T));
        //m_pISPLscResult = r_pISPLscResult;
    }
    else
    {
        CAM_LOGD_IF(m_bDebugEnable,"[%s()] get result from lsc mgr directly !", __FUNCTION__);
        m_pLscMgr->getLSCResultPool(&m_pISPLscResult);
    }

    //LSC R1
    ISP_MGR_LSC_T& rLsc = ISP_MGR_LSC_T::getInstance(m_eSensorDev);

    MBOOL rMgrOnOff = m_pISPLscResult.fgOnOff;

    if (m_DebugCTL_Disable.bLSC_R1
        || (!rMgrOnOff)
        || (!bProfileRequest))
    {
        rLsc.enableLsc(0, MFALSE);
    }
    else
    {
        rLsc.enableLsc(0, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_LSC_R1(%d), ADB_LSC_MGR(%d), ProfileRequest(%d), m_rIspCamInfo.u4Id(%d)",
        __FUNCTION__
        , rLsc.isEnable(0)
        , m_DebugCTL_Disable.bLSC_R1
        , rMgrOnOff
        , bProfileRequest
        , m_rIspCamInfo.u4Id);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_WB()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_WB_EN) ? MTRUE : MFALSE;

    //WB R1
    ISP_MGR_WB_T& rWb = ISP_MGR_WB_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bWB_R1
        || (!bProfileRequest)
        || (m_rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Capture))
    {
        rWb.setEnable(ISP_MGR_WB::EWB_R1, MFALSE);
    }
    else
    {
        rWb.setEnable(ISP_MGR_WB::EWB_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_WB_R1(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rWb.isEnable(ISP_MGR_WB::EWB_R1)
        , m_DebugCTL_Disable.bWB_R1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_HLR()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_HLR_EN) ? MTRUE : MFALSE;

    //HLR R1
    ISP_MGR_HLR_T& rHlr = ISP_MGR_HLR_T::getInstance(m_eSensorDev);

    MBOOL bLTM_EN = ISP_MGR_LTM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_LTM_T::ELTM_R1);

    if (m_DebugCTL_Disable.bHLR_R1
        || (!bProfileRequest)
        || (!bLTM_EN))
    {
        rHlr.setEnable(ISP_MGR_HLR::EHLR_R1, MFALSE);
    }
    else
    {
        rHlr.setEnable(ISP_MGR_HLR::EHLR_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_HLR_R1(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rHlr.isEnable(ISP_MGR_HLR::EHLR_R1)
        , m_DebugCTL_Disable.bHLR_R1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_LTM()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_LTM_EN) ? MTRUE : MFALSE;

    //LTM R1
    ISP_MGR_LTM_T& rLtm = ISP_MGR_LTM_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bLTM_R1
        || (!bProfileRequest))
    {
        rLtm.setEnable(ISP_MGR_LTM::ELTM_R1, MFALSE);
    }
    else
    {
        rLtm.setEnable(ISP_MGR_LTM::ELTM_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_LTM_R1(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rLtm.isEnable(ISP_MGR_LTM::ELTM_R1)
        , m_DebugCTL_Disable.bLTM_R1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_LCES()
{
    ISP_MGR_LCES_T& rLces = ISP_MGR_LCES_T::getInstance(m_eSensorDev);

    //LCES D1
    if (m_DebugCTL_Disable.bLCES_R1 ||
        (m_eOperMode == EOperMpde_Factory))
    {
        rLces.setEnable(ISP_MGR_LCES::ELCES_R1, MFALSE);
    }
    else
    {
        rLces.setEnable(ISP_MGR_LCES::ELCES_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_LCES_R1(%d), Factory Mode(%d)",
        __FUNCTION__
        , rLces.isEnable(ISP_MGR_LCES::ELCES_R1)
        , m_DebugCTL_Disable.bLCES_R1
        , (m_eOperMode == EOperMpde_Factory) ? MTRUE : MFALSE);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_SLK()
{

    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_SLK_EN) ? MTRUE : MFALSE;

    ISP_MGR_SLK_T& rSlk = ISP_MGR_SLK_T::getInstance(m_eSensorDev);

    MBOOL bShading_P1_En = ISP_MGR_LSC_T::getInstance(m_eSensorDev).isEnable(0);

    MBOOL bStereo_Path = (m_u4P1DirectYUV_PortInfo & EP1_Depth_Path);

    MBOOL bDirectYUV = bStereo_Path || ( m_u4P1DirectYUV_PortInfo & EP1_FD_Path);

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;


    //SLK R1
    if (m_DebugCTL_Disable.bSLK_R1
        || (!bProfileRequest)
        || (!bShading_P1_En)
        || (!bDirectYUV)
        || bRawC)
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_R1, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_R1, MTRUE);
    }

    //SLK R2
    if (m_DebugCTL_Disable.bSLK_R2
        || (!bProfileRequest)
        || (!bShading_P1_En)
        || (!bStereo_Path)
        || bRawC)
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_R2, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_R2, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] DirectYUV(%d), ADB_SLK_R1(%d), ADB_SLK_R2(%d), ProfileRequest(%d), bShading_P1_En(%d)",
        __FUNCTION__
        , bDirectYUV
        , m_DebugCTL_Disable.bSLK_R1
        , m_DebugCTL_Disable.bSLK_R2
        , bProfileRequest
        , bShading_P1_En);

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final R1(%d), R2(%d), YUV_Port(0x%x), RawC(%d)",
        __FUNCTION__
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_R1)
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_R2)
        , m_u4P1DirectYUV_PortInfo
        , bRawC);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_DM()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_DM_EN) ? MTRUE : MFALSE;

    MBOOL bDirectYUV = ( m_u4P1DirectYUV_PortInfo & EP1_FD_Path) | (m_u4P1DirectYUV_PortInfo & EP1_Depth_Path);

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;

    ISP_MGR_DM_T& rDm = ISP_MGR_DM_T::getInstance(m_eSensorDev);

    //DM R1
    if (m_DebugCTL_Disable.bDM_R1
        || (!bProfileRequest)
        || (!bDirectYUV)
        || bRawC)
    {
        rDm.setEnable(ISP_MGR_DM::EDM_R1, MFALSE);
    }
    else
    {
        rDm.setEnable(ISP_MGR_DM::EDM_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), m_u4P1DirectYUV_PortInfo(%d), ADB_DM_R1(%d), ProfileRequest(%d), RawC(%d)",
        __FUNCTION__
        , rDm.isEnable(ISP_MGR_DM::EDM_R1)
        , m_u4P1DirectYUV_PortInfo
        , m_DebugCTL_Disable.bDM_R1
        , bProfileRequest
        , bRawC);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_FLC()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_FLC_EN) ? MTRUE : MFALSE;

    MBOOL bDirectYUV = ( m_u4P1DirectYUV_PortInfo & EP1_FD_Path) | (m_u4P1DirectYUV_PortInfo & EP1_Depth_Path);

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;

    ISP_MGR_FLC_T& rFlc = ISP_MGR_FLC_T::getInstance(m_eSensorDev);

    //FLC R1
    if (m_DebugCTL_Disable.bFLC_R1
        || (!bProfileRequest)
        || (!bDirectYUV)
        || bRawC)
    {
        rFlc.setEnable(ISP_MGR_FLC::EFLC_R1, MFALSE);
    }
    else
    {
        rFlc.setEnable(ISP_MGR_FLC::EFLC_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), DirectYUV(%d), ADB_FLC_R1(%d), ProfileRequest(%d), RawC(%d)",
        __FUNCTION__
        , rFlc.isEnable(ISP_MGR_FLC::EFLC_R1)
        , bDirectYUV
        , m_DebugCTL_Disable.bFLC_R1
        , bProfileRequest
        , bRawC);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_CCM()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_CCM_EN) ? MTRUE : MFALSE;

    MBOOL bDirectYUV = ( m_u4P1DirectYUV_PortInfo & EP1_FD_Path) | (m_u4P1DirectYUV_PortInfo & EP1_Depth_Path);

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;

    MBOOL bMono = (m_u4RawFmtType == SENSOR_RAW_MONO) ? MTRUE : MFALSE;

    ISP_MGR_CCM_T& rCcm = ISP_MGR_CCM_T::getInstance(m_eSensorDev);

    //CCM R1
    if (m_DebugCTL_Disable.bCCM_R1
        || (!bProfileRequest)
        || (!bDirectYUV)
        || bRawC
        || bMono)
    {
        rCcm.setEnable(ISP_MGR_CCM::ECCM_R1, MFALSE);
    }
    else
    {
        rCcm.setEnable(ISP_MGR_CCM::ECCM_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), m_u4P1DirectYUV_PortInfo(%d), ADB_CCM_R1(%d), ProfileRequest(%d), RawC(%d), Mono(%d)",
    __FUNCTION__
    , rCcm.isEnable(ISP_MGR_CCM::ECCM_R1)
    , m_u4P1DirectYUV_PortInfo
    , m_DebugCTL_Disable.bCCM_R1
    , bProfileRequest
    , bRawC
    , bMono);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_GGM()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_GGM_EN) ? MTRUE : MFALSE;

    ISP_MGR_GGM_T& rGgm = ISP_MGR_GGM_T::getInstance(m_eSensorDev);

    MBOOL bStereo_Path = (m_u4P1DirectYUV_PortInfo & EP1_Depth_Path);
    MBOOL bFD_Path = ( m_u4P1DirectYUV_PortInfo & EP1_FD_Path);

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;

    //GGM R1
    if (m_DebugCTL_Disable.bGGM_R1
        || (!bProfileRequest)
        || (!bStereo_Path)
        || bRawC)
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_R1, MFALSE);
    }
    else
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_R1, MTRUE);
    }

    //GGM R2
    if (m_DebugCTL_Disable.bGGM_R2
        || (!bProfileRequest)
        || (!bFD_Path)
        || bRawC)
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_R2, MFALSE);
    }
    else
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_R2, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(R1,R2)=(%d,%d), ADB_GGM_R1(%d), ADB_GGM_R2(%d), ProfileRequest(%d), StereoPath(%d), FDPath(%d), RawC(%d)",
        __FUNCTION__
        , rGgm.isEnable(ISP_MGR_GGM::EGGM_R1)
        , rGgm.isEnable(ISP_MGR_GGM::EGGM_R2)
        , m_DebugCTL_Disable.bGGM_R1
        , m_DebugCTL_Disable.bGGM_R2
        , bProfileRequest
        , bStereo_Path
        , bFD_Path
        , bRawC);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_G2C()
{
    ISP_MGR_G2C_T& rG2c = ISP_MGR_G2C_T::getInstance(m_eSensorDev);

    MBOOL bStereo_Path = (m_u4P1DirectYUV_PortInfo & EP1_Depth_Path);
    MBOOL bFD_Path = ( m_u4P1DirectYUV_PortInfo & EP1_FD_Path);

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;

    //G2C R1
    if (m_DebugCTL_Disable.bG2C_R1
        || (!bStereo_Path)
        || bRawC)
    {
        rG2c.setEnable(ISP_MGR_G2C::EG2C_R1, MFALSE);
    }
    else
    {
        rG2c.setEnable(ISP_MGR_G2C::EG2C_R1, MTRUE);
    }

    //G2C R2
    if (m_DebugCTL_Disable.bG2C_R2
        || (!bFD_Path)
        || bRawC)
    {
        rG2c.setEnable(ISP_MGR_G2C::EG2C_R2, MFALSE);
    }
    else
    {
        rG2c.setEnable(ISP_MGR_G2C::EG2C_R2, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(R1,R2)=(%d,%d), ADB_G2C_R1(%d), ADB_G2C_R2(%d), StereoPath(%d), FDPath(%d), RawC(%d)",
        __FUNCTION__
        , rG2c.isEnable(ISP_MGR_G2C::EG2C_R1)
        , rG2c.isEnable(ISP_MGR_G2C::EG2C_R2)
        , m_DebugCTL_Disable.bG2C_R1
        , m_DebugCTL_Disable.bG2C_R2
        , bStereo_Path
        , bFD_Path
        , bRawC);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P1_YNRS()
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[m_rIspCamInfo.rMapping_Info.eIspProfile] & M_YNR_EN) ? MTRUE : MFALSE;

    MBOOL bStereo_Path = (m_u4P1DirectYUV_PortInfo & EP1_Depth_Path);

    MBOOL bRawC = (m_ERawPath == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C)? MTRUE : MFALSE;

    ISP_MGR_YNRS_T& rYnrs = ISP_MGR_YNRS_T::getInstance(m_eSensorDev);

    //YNRS R1
    if (m_DebugCTL_Disable.bYNRS_R1
        || (!bProfileRequest)
        || (!bStereo_Path)
        || bRawC)
    {
        rYnrs.setEnable(ISP_MGR_YNRS::EYNRS_R1, MFALSE);
    }
    else
    {
        rYnrs.setEnable(ISP_MGR_YNRS::EYNRS_R1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_YNRS_R1(%d), ProfileRequest(%d), StereoPath(%d), RawC(%d)",
        __FUNCTION__
        , rYnrs.isEnable(ISP_MGR_YNRS::EYNRS_R1)
        , m_DebugCTL_Disable.bYNRS_R1
        , bProfileRequest
        , bStereo_Path
        , bRawC);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_OBC(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_OBC_EN) ? MTRUE : MFALSE;

    //OBC D1
    ISP_MGR_OBC_T& rObc = ISP_MGR_OBC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bOBC_D1
        || (!rObc.isCCTEnable())
        || (!bProfileRequest)
        || rIspCamInfo.fgRPGEnable
        || (!fgPureRaw)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rObc.setEnable(ISP_MGR_OBC::EOBC_D1, MFALSE);
    }
    else
    {
        rObc.setEnable(ISP_MGR_OBC::EOBC_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_OBC_D1(%d), CCTEnable(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d)",
        __FUNCTION__
        , rObc.isEnable(ISP_MGR_OBC::EOBC_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bOBC_D1
        , rObc.isCCTEnable()
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_BPC(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_BPC_EN) ? MTRUE : MFALSE;

    //BPC_BPC_D1
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bBPC_BPC_D1
        || (!rBpc.isCCTBPCEnable())
        || (!bProfileRequest)
        || rIspCamInfo.fgRPGEnable
        || (!fgPureRaw)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rBpc.setBPCEnable(ISP_MGR_BPC::EBPC_D1, MFALSE);
    }
    else
    {
        rBpc.setBPCEnable(ISP_MGR_BPC::EBPC_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_BPC_D1(%d), CCTBPCEnable(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d)",
        __FUNCTION__
        , rBpc.isBPCEnable(ISP_MGR_BPC::EBPC_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bBPC_BPC_D1
        , rBpc.isCCTBPCEnable()
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_CT(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_CT_EN) ? MTRUE : MFALSE;

    //BPC_CT_D1
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bBPC_CT_D1
        || (!rBpc.isCCTCTEnable())
        || (!bProfileRequest)
        || rIspCamInfo.fgRPGEnable
        || (!fgPureRaw)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rBpc.setCTEnable(ISP_MGR_BPC::EBPC_D1, MFALSE);
    }
    else
    {
        rBpc.setCTEnable(ISP_MGR_BPC::EBPC_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_CT_D1(%d), CCTCTEnable(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d)",
        __FUNCTION__
        , rBpc.isCTEnable(ISP_MGR_BPC::EBPC_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bBPC_CT_D1
        , rBpc.isCCTCTEnable()
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_PDC(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_PDC_EN) ? MTRUE : MFALSE;

    //BPC_PDC_D1
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bBPC_PDC_D1
        || (!rBpc.isCCTPDCEnable())
        || (!bProfileRequest)
        || rIspCamInfo.fgRPGEnable
        || (!fgPureRaw)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rBpc.setPDCEnable(ISP_MGR_BPC::EBPC_D1, MFALSE);
    }
    else
    {
        Tbl P2BPCITbl_Info;
        if(IPDTblGen::getInstance()->getTbl( m_eSensorDev, rIspCamInfo.rMapping_Info.eSensorMode, P2BPCITbl_Info))
        {
            rBpc.setDMGItable(P2BPCITbl_Info.pbuf);
            rBpc.setBPCIValid(ISP_MGR_BPC::EBPC_D1, MTRUE);
            rBpc.setPDCEnable(ISP_MGR_BPC::EBPC_D1, MTRUE);
        }
        else{
            rBpc.setDMGItable(NULL);
            rBpc.setBPCIValid(ISP_MGR_BPC::EBPC_D1, MFALSE);
            rBpc.setPDCEnable(ISP_MGR_BPC::EBPC_D1, MFALSE);
#if 0
            CAM_LOGE("%s(): Fail to Gen PDC Table, SensorDev(%d), SensorMode(%d), FrontBin(%d)\n",
                __FUNCTION__,
                m_eSensorDev, rIspCamInfo.rMapping_Info.eSensorMode, rIspCamInfo.rMapping_Info.eFrontBin);
#endif
        }

    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_PDC_D1(%d), CCTPDCEnable(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d), BPCI(%p)",
        __FUNCTION__
        , rBpc.isPDCEnable(ISP_MGR_BPC::EBPC_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bBPC_PDC_D1
        , rBpc.isCCTPDCEnable()
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw
        , rBpc.getDMGItable());

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_DGN(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_DGN_EN) ? MTRUE : MFALSE;

    //DGN D1
    ISP_MGR_DGN_T& rDgn = ISP_MGR_DGN_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bDGN_D1
        || (!bProfileRequest)
        || rIspCamInfo.fgRPGEnable
        || (!fgPureRaw)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rDgn.setEnable(ISP_MGR_DGN::EDGN_D1, MFALSE);
    }
    else
    {
        rDgn.setEnable(ISP_MGR_DGN::EDGN_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_DGN_D1(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d)",
        __FUNCTION__
        , rDgn.isEnable(ISP_MGR_DGN::EDGN_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bDGN_D1
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_LSC(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPgnEn = !rIspCamInfo.fgRPGEnable;
    MBOOL fgPureRaw = rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure; //only for IMGO
    EIspProfile_T profile = rIspCamInfo.rMapping_Info.eIspProfile;
    CAM_LOGD_IF(m_bDebugEnable, "[%s] Profile = %d", __FUNCTION__, profile);
    MBOOL fgLsc2En = fgPureRaw && fgPgnEn;

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_LSC_EN) ? MTRUE : MFALSE;

    //OBC D1
    ISP_MGR_LSC_T& rLsc2 = ISP_MGR_LSC_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bLSC_D1
        || (!bProfileRequest)
        || (!fgLsc2En)
        || (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_YUV_Reprocess)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rLsc2.enableLsc(1, MFALSE);
        m_pLscMgr->setOnOffP2(MFALSE);
    }
    else
    {
        rLsc2.enableLsc(1, MTRUE);
        m_pLscMgr->setOnOffP2(MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), Profile = %d, i4P2InImgFmt(%d), ADB_LSC_D1(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d)",
        __FUNCTION__
        , rLsc2.isEnable(1)
        , profile
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bLSC_D1
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_WB(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_WB_EN) ? MTRUE : MFALSE;

    //WB D1
    ISP_MGR_WB_T& rWb = ISP_MGR_WB_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bWB_D1
        || (!bProfileRequest)
        || rIspCamInfo.fgRPGEnable
        || ((!fgPureRaw) && (rIspCamInfo.rMapping_Info.eIspProfile != EIspProfile_Auto_mHDR_Capture))
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rWb.setEnable(ISP_MGR_WB::EWB_D1, MFALSE);
    }
    else
    {
        rWb.setEnable(ISP_MGR_WB::EWB_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_WB_D1(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d)",
        __FUNCTION__
        , rWb.isEnable(ISP_MGR_WB::EWB_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bWB_D1
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_LTM(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_LTM_EN) ? MTRUE : MFALSE;

    //LTM D1
    ISP_MGR_LTM_T& rLtm = ISP_MGR_LTM_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bLTM_D1
        || (!bProfileRequest)
        || (rIspCamInfo.fgRPGEnable)
        || (!fgPureRaw)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv)
        || (!rIspCamInfo.rCCU_Result.LTM.LTM_Valid)
        || (!rIspCamInfo.rCCU_Result.LTM.LTM_EN)
        || (rIspCamInfo.rMapping_Info.eFlash)
        || (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE))
    {
        rLtm.setEnable(ISP_MGR_LTM::ELTM_D1, MFALSE);
    }
    else
    {
        rLtm.setEnable(ISP_MGR_LTM::ELTM_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_LTM_D1(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d), CCU_Query(%d), eFlash(%d)",
        __FUNCTION__
        , rLtm.isEnable(ISP_MGR_LTM::ELTM_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bLTM_D1
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw
        , rIspCamInfo.rCCU_Result.LTM.LTM_Valid
        , rIspCamInfo.rMapping_Info.eFlash);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_HLR(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL fgPureRaw = (rIspCamInfo.i4RawType == NSIspTuning::ERawType_Pure); //only for IMGO

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_HLR_EN) ? MTRUE : MFALSE;

    //HLR D1
    ISP_MGR_HLR_T& rHlr = ISP_MGR_HLR_T::getInstance(m_eSensorDev);

    MBOOL bLTM_EN = ISP_MGR_LTM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_LTM_T::ELTM_D1);

    if (m_DebugCTL_Disable.bHLR_D1
        || (!bProfileRequest)
        || rIspCamInfo.fgRPGEnable
        || (!fgPureRaw)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv)
        || (!rIspCamInfo.rCCU_Result.HLR.HLR_Valid)
        || (!rIspCamInfo.rCCU_Result.HLR.HLR_EN)
        || (!bLTM_EN))
    {
        rHlr.setEnable(ISP_MGR_HLR::EHLR_D1, MFALSE);
    }
    else
    {
        rHlr.setEnable(ISP_MGR_HLR::EHLR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_HLR_D1(%d), ProfileRequest(%d), RPG_En(%d), PureRaw(%d), CCU_Query(%d), LTM_D1_EN(%d)",
        __FUNCTION__
        , rHlr.isEnable(ISP_MGR_HLR::EHLR_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bHLR_D1
        , bProfileRequest
        , rIspCamInfo.fgRPGEnable
        , fgPureRaw
        , rIspCamInfo.rCCU_Result.HLR.HLR_Valid
        , bLTM_EN);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_SLK(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_SLK_EN) ? MTRUE : MFALSE;

    ISP_MGR_SLK_T& rSlk = ISP_MGR_SLK_T::getInstance(m_eSensorDev);

    MBOOL bShading_P1_En = ((rIspCamInfo.i4RawType == ERawType_Proc) || rIspCamInfo.fgRPGEnable)
                        && rIspCamInfo.rSl2Info.bLSC_R1_En ;

    MBOOL bShading_P2_En = ISP_MGR_LSC_T::getInstance(m_eSensorDev).isEnable(1);

    MBOOL bIsYuvShadingEn = bProfileRequest  && (rIspCamInfo.i4P2InImgFmt == 1) && !m_DebugCTL_Disable.bLSC_D1;

    //SLK D1
    if (m_DebugCTL_Disable.bSLK_D1
        || (!bProfileRequest)
        || ((!bShading_P1_En) &&(!bShading_P2_En))
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D1, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D1, MTRUE);
    }

    //SLK D2
    if (bIsYuvShadingEn)
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D2, MTRUE);
    }
    else if (m_DebugCTL_Disable.bSLK_D2
        || (!bProfileRequest)
        || ((!bShading_P1_En) &&(!bShading_P2_En)))
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D2, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D2, MTRUE);
    }

    //SLK D3
    if (bIsYuvShadingEn)
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D3, MTRUE);
    }
    else if (m_DebugCTL_Disable.bSLK_D3
        || (!bProfileRequest)
        || ((!bShading_P1_En) &&(!bShading_P2_En)))
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D3, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D3, MTRUE);
    }

    //SLK D4
    if (bIsYuvShadingEn)
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D4, MTRUE);
    }
    else if (m_DebugCTL_Disable.bSLK_D4
        || (!bProfileRequest)
        || ((!bShading_P1_En) &&(!bShading_P2_En)))
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D4, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D4, MTRUE);
    }

    //SLK D5
    if (bIsYuvShadingEn)
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D5, MTRUE);
    }
    else if (m_DebugCTL_Disable.bSLK_D5
        || (!bProfileRequest)
        || ((!bShading_P1_En) &&(!bShading_P2_En)))
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D5, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D5, MTRUE);
    }

    //SLK D6
    if (bIsYuvShadingEn)
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D6, MTRUE);
    }
    else if (m_DebugCTL_Disable.bSLK_D6
        || (!bProfileRequest)
        || ((!bShading_P1_En) &&(!bShading_P2_En)))
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D6, MFALSE);
    }
    else
    {
        rSlk.setEnable(ISP_MGR_SLK::ESLK_D6, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()]D1(%d), D2(%d), D3(%d), D4(%d), D5(%d), D6(%d)",
        __FUNCTION__
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_D1)
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_D2)
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_D3)
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_D4)
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_D5)
        , rSlk.isEnable(ISP_MGR_SLK::ESLK_D6));


    CAM_LOGD_IF(m_bDebugEnable,"[%s()] i4P2InImgFmt(%d), ADB_SLK_D1(%d), ADB_SLK_D2(%d), ADB_SLK_D3(%d), ADB_SLK_D4(%d), ADB_SLK_D5(%d), ADB_SLK_D6(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bSLK_D1
        , m_DebugCTL_Disable.bSLK_D2
        , m_DebugCTL_Disable.bSLK_D3
        , m_DebugCTL_Disable.bSLK_D4
        , m_DebugCTL_Disable.bSLK_D5
        , m_DebugCTL_Disable.bSLK_D6
        , bProfileRequest);

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] i4RawType(%d), fgRPGEnable(%d), rSl2Info.bLSC_R1_En(%d), bShading_P1_En(%d), bShading_P2_En(%d), bIsYuvShadingEn(%d)",
        __FUNCTION__
        , rIspCamInfo.i4RawType
        , rIspCamInfo.fgRPGEnable
        , rIspCamInfo.rSl2Info.bLSC_R1_En
        , bShading_P1_En
        , bShading_P2_En
        , bIsYuvShadingEn);


    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_DM(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_DM_EN) ? MTRUE : MFALSE;

    ISP_MGR_DM_T& rDm = ISP_MGR_DM_T::getInstance(m_eSensorDev);

    MBOOL bMono = (m_u4RawFmtType == SENSOR_RAW_MONO) ? MTRUE : MFALSE;
    rDm.setMono(bMono);

    //DM D1
    if (m_DebugCTL_Disable.bDM_D1
        || (!rDm.isCCTEnable())
        || (!bProfileRequest)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rDm.setEnable(ISP_MGR_DM::EDM_D1, MFALSE);
    }
    else
    {
        rDm.setEnable(ISP_MGR_DM::EDM_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_DM_D1(%d), CCTEnable(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rDm.isEnable(ISP_MGR_DM::EDM_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bDM_D1
        , rDm.isCCTEnable()
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_LDNR(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_LDNR_EN) ? MTRUE : MFALSE;

    ISP_MGR_LDNR_T& rLdnr = ISP_MGR_LDNR_T::getInstance(m_eSensorDev);

    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;

    MBOOL bNRModeDisableControl = (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL);

    MBOOL bMono = (m_u4RawFmtType == SENSOR_RAW_MONO) ? MTRUE : MFALSE;

    //LDNR D1
    if (m_DebugCTL_Disable.bLDNR_D1
        || (!rLdnr.isCCTEnable())
        || (!bProfileRequest)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv)
        || (bNRModeDisableControl)
        || bMono)
    {
        rLdnr.setEnable(ISP_MGR_LDNR::ELDNR_D1, MFALSE);
    }
    else
    {
        rLdnr.setEnable(ISP_MGR_LDNR::ELDNR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_LDNR_D1(%d), CCTEnable(%d), ProfileRequest(%d), bNRModeDisableControl(%d), MONO(%d)",
        __FUNCTION__
        , rLdnr.isEnable(ISP_MGR_LDNR::ELDNR_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bLDNR_D1
        , rLdnr.isCCTEnable()
        , bProfileRequest
        , bNRModeDisableControl
        , bMono);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_FLC(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_FLC_EN) ? MTRUE : MFALSE;

    ISP_MGR_FLC_T& rFlc = ISP_MGR_FLC_T::getInstance(m_eSensorDev);

    //FLC D1
    if (m_DebugCTL_Disable.bFLC_D1
        || (!bProfileRequest)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rFlc.setEnable(ISP_MGR_FLC::EFLC_D1, MFALSE);
    }
    else
    {
        rFlc.setEnable(ISP_MGR_FLC::EFLC_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), i4P2InImgFmt(%d), ADB_FLC_D1(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rFlc.isEnable(ISP_MGR_FLC::EFLC_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bFLC_D1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_CCM(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_CCM_EN) ? MTRUE : MFALSE;

    MBOOL bStereo_Request = ( (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview) ||
                              (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW)) ? MTRUE : MFALSE;

    MBOOL bMono = (m_u4RawFmtType == SENSOR_RAW_MONO) ? MTRUE : MFALSE;

    ISP_MGR_CCM_T& rCcm = ISP_MGR_CCM_T::getInstance(m_eSensorDev);

    //CCM D1
    if (m_DebugCTL_Disable.bCCM_D1
        || (!rCcm.isCCTEnable())
        || (!bProfileRequest)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv)
        || bMono)
    {
        rCcm.setEnable(ISP_MGR_CCM::ECCM_D1, MFALSE);
    }
    else
    {
        rCcm.setEnable(ISP_MGR_CCM::ECCM_D1, MTRUE);
    }

    //CCM D2
    if (m_DebugCTL_Disable.bCCM_D2
        || (!rCcm.isCCTEnable())
        || (!bProfileRequest)
        || (!bStereo_Request)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv)
        || bMono)
    {
        rCcm.setEnable(ISP_MGR_CCM::ECCM_D2, MFALSE);
    }
    else
    {
        rCcm.setEnable(ISP_MGR_CCM::ECCM_D2, MTRUE);
    }

    //CCM D3 always bypass
    rCcm.setEnable(ISP_MGR_CCM::ECCM_D3, MFALSE);

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(D1,D2,D3)=(%d,%d,%d), i4P2InImgFmt(%d), ADB_CCM_D1(%d), ADB_CCM_D2(%d), ADB_CCM_D3(%d), ProfileRequest(%d), StereoPath(%d), Mono(%d)",
        __FUNCTION__
        , rCcm.isEnable(ISP_MGR_CCM::ECCM_D1)
        , rCcm.isEnable(ISP_MGR_CCM::ECCM_D2)
        , rCcm.isEnable(ISP_MGR_CCM::ECCM_D3)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bCCM_D1
        , m_DebugCTL_Disable.bCCM_D2
        , m_DebugCTL_Disable.bCCM_D3
        , bProfileRequest
        , bStereo_Request
        , bMono);


    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_GGM(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_GGM_EN) ? MTRUE : MFALSE;

    MBOOL bStereo_Request = ( (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview) ||
                              (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW)) ? MTRUE : MFALSE;

    ISP_MGR_GGM_T& rGgm = ISP_MGR_GGM_T::getInstance(m_eSensorDev);

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] i4P2InImgFmt(%d), ADB_GGM_D1(%d), ADB_GGM_D2(%d), ADB_GGM_D3(%d), ADB_IGGM_D1(%d), ProfileRequest(%d), StereoPath(%d)",
        __FUNCTION__
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bGGM_D1
        , m_DebugCTL_Disable.bGGM_D2
        , m_DebugCTL_Disable.bGGM_D3
        , m_DebugCTL_Disable.bIGGM_D1
        , bProfileRequest
        , bStereo_Request);

    //GGM D1
    if (m_DebugCTL_Disable.bGGM_D1
        || (!rGgm.isCCTEnable())
        || (!bProfileRequest)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv)
        || (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE))
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_D1, MFALSE);
    }
    else
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_D1, MTRUE);
    }

    //GGM D2
    if (m_DebugCTL_Disable.bGGM_D2
        || (!rGgm.isCCTEnable())
        || (!bProfileRequest)
        || (!bStereo_Request)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_D2, MFALSE);
    }
    else
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_D2, MTRUE);
    }

    //GGM D3
    if (m_DebugCTL_Disable.bGGM_D3
        || (!rGgm.isCCTEnable())
        || (!bProfileRequest)
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_D3, MFALSE);
    }
    else
    {
        rGgm.setEnable(ISP_MGR_GGM::EGGM_D3, MTRUE);
    }

    //IGGM D1
    ISP_MGR_IGGM_T& rIggm = ISP_MGR_IGGM_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bIGGM_D1
        || (!rGgm.isEnable(ISP_MGR_GGM::EGGM_D3))
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv)
        || (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE))
    {
        rIggm.setEnable(ISP_MGR_IGGM::EIGGM_D1, MFALSE);
    }
    else
    {
        rIggm.setEnable(ISP_MGR_IGGM::EIGGM_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(D1,D2,D3,IGGM)=(%d,%d,%d,%d), iGGMFinali4P2InImgFmt(%d), ADB_GGM_D1(%d), ADB_GGM_D2(%d), ADB_GGM_D3(%d), ADB_IGGM_D1(%d), ProfileRequest(%d), StereoPath(%d), eToneMapMode(%d)",
        __FUNCTION__
        , rGgm.isEnable(ISP_MGR_GGM::EGGM_D1)
        , rGgm.isEnable(ISP_MGR_GGM::EGGM_D2)
        , rGgm.isEnable(ISP_MGR_GGM::EGGM_D3)
        , rIggm.isEnable(ISP_MGR_IGGM::EIGGM_D1)
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bGGM_D1
        , m_DebugCTL_Disable.bGGM_D2
        , m_DebugCTL_Disable.bGGM_D3
        , m_DebugCTL_Disable.bIGGM_D1
        , bProfileRequest
        , bStereo_Request
        , rIspCamInfo.eToneMapMode);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_G2C(const RAWIspCamInfo& rIspCamInfo)
{

    //G2C D1
    ISP_MGR_G2C_T& rG2c = ISP_MGR_G2C_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bG2C_D1)
    {
        rG2c.setEnable(ISP_MGR_G2C::EG2C_D1, MFALSE);
    }
    else
    {
        rG2c.setEnable(ISP_MGR_G2C::EG2C_D1, MTRUE);
    }

    //C2G D1
    ISP_MGR_C2G_T& rC2g = ISP_MGR_C2G_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bC2G_D1
        || (!rG2c.isEnable(ISP_MGR_G2C::EG2C_D1)))
    {
        rC2g.setEnable(ISP_MGR_C2G::EC2G_D1, MFALSE);
    }
    else
    {
        rC2g.setEnable(ISP_MGR_C2G::EC2G_D1, MTRUE);
    }

    //G2CX D1
    ISP_MGR_G2CX_T& rG2cx = ISP_MGR_G2CX_T::getInstance(m_eSensorDev);

    if (m_DebugCTL_Disable.bG2CX_D1
        || (rIspCamInfo.i4P2InImgFmt == EYuv2Yuv))
    {
        rG2cx.setEnable(ISP_MGR_G2CX::EG2CX_D1, MFALSE);
    }
    else
    {
        rG2cx.setEnable(ISP_MGR_G2CX::EG2CX_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final G2C_D1(%d), C2G_D1(%d), G2CX_D1(%d)",
        __FUNCTION__
        , rG2c.isEnable(ISP_MGR_G2C::EG2C_D1)
        , rC2g.isEnable(ISP_MGR_C2G::EC2G_D1)
        , rG2cx.isEnable(ISP_MGR_G2CX::EG2CX_D1));

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] i4P2InImgFmt(%d), ADB_G2C_D1(%d), ADB_C2G_D1(%d), ADB_G2CX_D1(%d)",
        __FUNCTION__
        , rIspCamInfo.i4P2InImgFmt
        , m_DebugCTL_Disable.bG2C_D1
        , m_DebugCTL_Disable.bC2G_D1
        , m_DebugCTL_Disable.bG2CX_D1);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_YNR(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_YNR_EN) ? MTRUE : MFALSE;
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    MBOOL bNRModeDisableControl = (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL);

    ISP_MGR_YNR_T& rYnr = ISP_MGR_YNR_T::getInstance(m_eSensorDev);


    //YNR D1
    if (m_DebugCTL_Disable.bYNR_D1
        || (!rYnr.isCCTEnable())
        || (!bProfileRequest)
        || bNRModeDisableControl)
    {
        rYnr.setEnable(ISP_MGR_YNR::EYNR_D1, MFALSE);
    }
    else
    {
        rYnr.setEnable(ISP_MGR_YNR::EYNR_D1, MTRUE);

    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_YNR_D1(%d), CCTEnable(%d), ProfileRequest(%d) bNRodeDisableControl(%d) NRmode(%d)",
        __FUNCTION__
        , rYnr.isEnable(ISP_MGR_YNR::EYNR_D1)
        , m_DebugCTL_Disable.bYNR_D1
        , rYnr.isCCTEnable()
        , bProfileRequest
        , bNRModeDisableControl
        , rIspCamInfo.eNRMode);

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_YNR_LINK(const RAWIspCamInfo& rIspCamInfo)
{
//Chooo, should be reviewed (wait for MFNR)

    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_YNR_LINK_EN) ? MTRUE : MFALSE;
    ISP_MGR_YNR_T& rYnr = ISP_MGR_YNR_T::getInstance(m_eSensorDev);
    MBOOL bYnrEnable = rYnr.isEnable(ISP_MGR_YNR::EYNR_D1);

    //YNR D1
    if (m_DebugCTL_Disable.bYNR_D1_LINK
        || (!bProfileRequest)
        || (!bYnrEnable))
    {
        rYnr.setSkinEnable(ISP_MGR_YNR::EYNR_D1, MFALSE);
    }
    else
    {
        rYnr.setSkinEnable(ISP_MGR_YNR::EYNR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_YNR_D1_LINK(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rYnr.isSkinEnable(ISP_MGR_YNR::EYNR_D1)
        , m_DebugCTL_Disable.bYNR_D1_LINK
        , bProfileRequest);


    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_LCE(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_LCE_EN) ? MTRUE : MFALSE;

    ISP_MGR_LCE_T& rLce = ISP_MGR_LCE_T::getInstance(m_eSensorDev);

    //LCE D1
    if (m_DebugCTL_Disable.bLCE_D1
        || (!bProfileRequest)
        || (m_pLCESBuffer == NULL)
        || (rIspCamInfo.bBypassLCE)
        || (rIspCamInfo.rLCS_Info.bLCESEnable == MFALSE)
        || (rIspCamInfo.rLCS_Info.u4OutWidth  == 0)
        || (rIspCamInfo.rLCS_Info.u4OutHeight == 0)
        || (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE))
    {
        rLce.setEnable(ISP_MGR_LCE::ELCE_D1, MFALSE);
    }
    else
    {
        rLce.setEnable(ISP_MGR_LCE::ELCE_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_LCE_D1(%d), ProfileRequest(%d), LCES_Invalid(%d), BypassLCE(%d), LCE_Info_enable(%d), LCSO(%d, %d)",
        __FUNCTION__
        , rLce.isEnable(ISP_MGR_LCE::ELCE_D1)
        , m_DebugCTL_Disable.bLCE_D1
        , bProfileRequest
        , (m_pLCESBuffer == NULL)
        , rIspCamInfo.bBypassLCE
        , rIspCamInfo.rLCS_Info.bLCESEnable
        , rIspCamInfo.rLCS_Info.u4OutWidth
        , rIspCamInfo.rLCS_Info.u4OutHeight);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_DCES(const RAWIspCamInfo& rIspCamInfo)
{
    ISP_MGR_DCES_T& rDces = ISP_MGR_DCES_T::getInstance(m_eSensorDev);

    //DCES D1
    if (m_DebugCTL_Disable.bDCES_D1)
    {
        rDces.setEnable(ISP_MGR_DCES::EDCES_D1, MFALSE);
    }
    else
    {
        rDces.setEnable(ISP_MGR_DCES::EDCES_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_DCES_D1(%d)",
        __FUNCTION__
        , rDces.isEnable(ISP_MGR_DCES::EDCES_D1)
        , m_DebugCTL_Disable.bDCES_D1);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_DCE(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_DCE_EN) ? MTRUE : MFALSE;

    ISP_MGR_DCE_T& rDce = ISP_MGR_DCE_T::getInstance(m_eSensorDev);
    MBOOL bManualDisable = (rIspCamInfo.eControlMode == MTK_CONTROL_MODE_OFF);

    //DCE D1
    if (m_DebugCTL_Disable.bDCE_D1
        || (!bProfileRequest)
        || (bManualDisable))
    {
        rDce.setEnable(ISP_MGR_DCE::EDCE_D1, MFALSE);
    }
    else
    {
        rDce.setEnable(ISP_MGR_DCE::EDCE_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] ADB_DCE_D1(%d), ProfileRequest(%d), DCES_Invalid(%d)",
        __FUNCTION__
        , rDce.isEnable(ISP_MGR_DCE::EDCE_D1)
        , m_DebugCTL_Disable.bDCE_D1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_EE(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_EE_EN) ? MTRUE : MFALSE;
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    MBOOL bEdgeModeDisableControl = (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF) || (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG);

    ISP_MGR_EE_T& rEe = ISP_MGR_EE_T::getInstance(m_eSensorDev);

    //EE D1
    if (m_DebugCTL_Disable.bEE_D1
        || (!rEe.isCCTEnable())
        || (!bProfileRequest)
        || bEdgeModeDisableControl)
    {
        rEe.setEnable(ISP_MGR_EE::EEE_D1, MFALSE);
    }
    else
    {
        rEe.setEnable(ISP_MGR_EE::EEE_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_EE_D1(%d), CCTEnable(%d), ProfileRequest(%d), bEdgeModeDisableControl(%d) edgemode(%d), eIdx_Edge(%d)",
        __FUNCTION__
        , rEe.isEnable(ISP_MGR_EE::EEE_D1)
        , m_DebugCTL_Disable.bEE_D1
        , rEe.isCCTEnable()
        , bProfileRequest
        , bEdgeModeDisableControl
        , rIspCamInfo.eEdgeMode
        , rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_CNR(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_CNR_EN) ? MTRUE : MFALSE;

    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    MBOOL bNRModeDisableControl = (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL);

    //CNR_CNR D1
    if (m_DebugCTL_Disable.bCNR_CNR_D1
        || (!rCnr.isCCTCNREnable())
        || (!bProfileRequest)
        || (bNRModeDisableControl))
    {
        rCnr.setCNREnable(ISP_MGR_CNR::ECNR_D1, MFALSE);
    }
    else
    {
        rCnr.setCNREnable(ISP_MGR_CNR::ECNR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_CNR_D1(%d), CCTCNREnable(%d), ProfileRequest(%d), bNRModeDisableControl(%d)",
        __FUNCTION__
        , rCnr.isCNREnable(ISP_MGR_CNR::ECNR_D1)
        , m_DebugCTL_Disable.bCNR_CNR_D1
        , rCnr.isCCTCNREnable()
        , bProfileRequest
        , bNRModeDisableControl);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_CCR(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_CCR_EN) ? MTRUE : MFALSE;

    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    MBOOL bNRModeDisableControl = (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL);

    //CNR_CCR D1
    if (m_DebugCTL_Disable.bCNR_CCR_D1
        || (!rCnr.isCCTCCREnable())
        || (!bProfileRequest)
        || (bNRModeDisableControl))
    {
        rCnr.setCCREnable(ISP_MGR_CNR::ECNR_D1, MFALSE);
    }
    else
    {
        rCnr.setCCREnable(ISP_MGR_CNR::ECNR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_CCR_D1(%d), CCTCCREnable(%d), ProfileRequest(%d), bNRModeDisableControl($d)",
        __FUNCTION__
        , rCnr.isCCREnable(ISP_MGR_CNR::ECNR_D1)
        , m_DebugCTL_Disable.bCNR_CCR_D1
        , rCnr.isCCTCCREnable()
        , bProfileRequest
        , bNRModeDisableControl);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_ABF(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_ABF_EN) ? MTRUE : MFALSE;

    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);

    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;
    MBOOL bNRModeDisableControl = (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_MINIMAL);

    //CNR_ABF_D1
    if (m_DebugCTL_Disable.bCNR_ABF_D1
        || (!rCnr.isCCTABFEnable())
        || (!bProfileRequest)
        || (bNRModeDisableControl))
    {
        rCnr.setABFEnable(ISP_MGR_CNR::ECNR_D1, MFALSE);
    }
    else
    {
        rCnr.setABFEnable(ISP_MGR_CNR::ECNR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_ABF_D1(%d), CCTEnable(%d), ProfileRequest(%d), bNRModeDisableControl(%d)",
        __FUNCTION__
        , rCnr.isABFEnable(ISP_MGR_CNR::ECNR_D1)
        , m_DebugCTL_Disable.bCNR_ABF_D1
        , rCnr.isCCTABFEnable()
        , bProfileRequest
        , bNRModeDisableControl);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_BOK(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_BOK_EN) ? MTRUE : MFALSE;

    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);

    //CNR_BOK_D1
    if (m_DebugCTL_Disable.bCNR_BOK_D1
        || (!bProfileRequest))
    {
        rCnr.setBOKEnable(ISP_MGR_CNR::ECNR_D1, MFALSE);
    }
    else
    {
        rCnr.setBOKEnable(ISP_MGR_CNR::ECNR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_BOK_D1(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rCnr.isBOKEnable(ISP_MGR_CNR::ECNR_D1)
        , m_DebugCTL_Disable.bCNR_BOK_D1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_NDG(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_NDG_EN) ? MTRUE : MFALSE;

    ISP_MGR_NDG_T& rNdg = ISP_MGR_NDG_T::getInstance(m_eSensorDev);

    ISP_MGR_YNR_T& rYnr = ISP_MGR_YNR_T::getInstance(m_eSensorDev);

    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);

    //NDG D1
    if (m_DebugCTL_Disable.bNDG_D1
        ||(!rYnr.isEnable(ISP_MGR_YNR::EYNR_D1))
        || (!bProfileRequest))
    {
    //Chooo, HW constraint, YNR real enable result after apply, so if YNR turn off, turn off NDG in HW constratint
        rNdg.setEnable(ISP_MGR_NDG::ENDG_D1, MFALSE);
    }
    else
    {
        rNdg.setEnable(ISP_MGR_NDG::ENDG_D1, MTRUE);
    }

    //NDG D2
    if (m_DebugCTL_Disable.bNDG_D2
        ||(!rCnr.isCNREnable(ISP_MGR_CNR::ECNR_D1))
        || (!bProfileRequest))
    {
    //Chooo, HW constraint, CNR real enable  result after apply, so if CNR turn off, turn off NDG2 in HW constratint
        rNdg.setEnable(ISP_MGR_NDG::ENDG_D2, MFALSE);
    }
    else
    {
        rNdg.setEnable(ISP_MGR_NDG::ENDG_D2, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(D1,D2)=(%d,%d), ADB_NDG_D1(%d), ADB_NDG_D2(%d), YNR_EN(%d), CNR_EN(%d) bProfileRequest(%d)",
        __FUNCTION__
        , rNdg.isEnable(ISP_MGR_NDG::ENDG_D1)
        , rNdg.isEnable(ISP_MGR_NDG::ENDG_D2)
        , m_DebugCTL_Disable.bNDG_D1
        , m_DebugCTL_Disable.bNDG_D2
        , rYnr.isEnable(ISP_MGR_YNR::EYNR_D1)
        , rCnr.isCNREnable(ISP_MGR_CNR::ECNR_D1)
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_COLOR(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_COLOR_EN) ? MTRUE : MFALSE;

    ISP_MGR_COLOR_T& rColor = ISP_MGR_COLOR_T::getInstance(m_eSensorDev);

    //COLOR D1
    if (m_DebugCTL_Disable.bCOLOR_D1
        || (!rColor.isCCTEnable())
        || (!bProfileRequest))
    {
        rColor.setEnable(ISP_MGR_COLOR::ECOLOR_D1, MFALSE);
    }
    else
    {
        rColor.setEnable(ISP_MGR_COLOR::ECOLOR_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_COLOR_D1(%d), CCTEnable(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rColor.isEnable(ISP_MGR_COLOR::ECOLOR_D1)
        , m_DebugCTL_Disable.bCOLOR_D1
        , rColor.isCCTEnable()
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_NR3D(const RAWIspCamInfo& rIspCamInfo)
{
    ISP_MGR_NR3D_T& rNr3d = ISP_MGR_NR3D_T::getInstance(m_eSensorDev);

    //NR3D D1
    if (m_DebugCTL_Disable.bNR3D_D1
        || (!rIspCamInfo.NR3D_Data.configInfo.enable))
    {
        rNr3d.setEnable(ISP_MGR_NR3D::ENR3D_D1, MFALSE);
    }
    else
    {
        rNr3d.setEnable(ISP_MGR_NR3D::ENR3D_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_NR3D_D1(%d), FeatureRequest(%d)",
        __FUNCTION__
        , rNr3d.isEnable(ISP_MGR_NR3D::ENR3D_D1)
        , m_DebugCTL_Disable.bNR3D_D1
        , rIspCamInfo.NR3D_Data.configInfo.enable);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_MFB(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_MFB_EN) ? MTRUE : MFALSE;

    ISP_MGR_MFB_T& rMfb = ISP_MGR_MFB_T::getInstance(m_eSensorDev);

    //MFB_D1
    if (m_DebugCTL_Disable.bMFB_D1
        || (!bProfileRequest))
    {
        rMfb.setEnable(ISP_MGR_MFB::EMFB_D1, MFALSE);
    }
    else
    {
        rMfb.setEnable(ISP_MGR_MFB::EMFB_D1, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_MFB_D1(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rMfb.isEnable(ISP_MGR_MFB::EMFB_D1)
        , m_DebugCTL_Disable.bMFB_D1
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_MIX(const RAWIspCamInfo& rIspCamInfo)
{
    MBOOL bProfileRequest = (m_pFeatureCtrl[rIspCamInfo.rMapping_Info.eIspProfile] & M_MIX_EN) ? MTRUE : MFALSE;

    ISP_MGR_MIX_T& rMix = ISP_MGR_MIX_T::getInstance(m_eSensorDev);

    //MIX_D1
    if (m_DebugCTL_Disable.bMIX_D3
        || (!bProfileRequest))
    {
        rMix.setEnable(ISP_MGR_MIX::EMIX_D3, MFALSE);
    }
    else
    {
        rMix.setEnable(ISP_MGR_MIX::EMIX_D3, MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_MIX_D3(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rMix.isEnable(ISP_MGR_MIX::EMIX_D3)
        , m_DebugCTL_Disable.bMIX_D3
        , bProfileRequest);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
decision_PerFrame_P2_LPCNR(const RAWIspCamInfo& rIspCamInfo)
{
    ISP_MGR_LPCNR_T& rLpcnr = ISP_MGR_LPCNR_T::getInstance(m_eSensorDev);
    rLpcnr.setEnable(ISP_MGR_LPCNR::ELPCNR_D1, MFALSE);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
get_P1_AllIndex(const RAWIspCamInfo& rIspCamInfo)
{

    IDXCACHE_STAGE eStage = IDXCACHE_STAGE_P1;

    MUINT16 idxL = 0, idxU = 0;

    if(ISP_MGR_OBC_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_OBC::EOBC_R1)
        || ISP_MGR_MOBC_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_MOBC::EMOBC_R2))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_OBC, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_OBC, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).isBPCEnable(ISP_MGR_BPC::EBPC_R1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_BPC_BPC, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_BPC_BPC, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).isCTEnable(ISP_MGR_BPC::EBPC_R1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_BPC_CT, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_BPC_CT, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).isPDCEnable(ISP_MGR_BPC::EBPC_R1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_BPC_PDC, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_BPC_PDC, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_DM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_DM::EDM_R1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_DM, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_DM, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_GGM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_GGM::EGGM_R1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_CURRENT, rIspCamInfo, EModule_GMA, eStage);

    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_YNRS_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_YNRS::EYNRS_R1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_YNRS, eStage);
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_YNR_TBL, eStage);

        if(isIspInterpolation(rIspCamInfo)){
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_YNRS, eStage);
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_YNR_TBL, eStage);
        }
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_CCM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_CCM::ECCM_R1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERLV , rIspCamInfo, EModule_CCM, eStage);
        idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERLV , rIspCamInfo, EModule_CCM, eStage);
    }
    else{
        //set cache valid = 0
    }

#if 0

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_MIX_D3(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rMix.isEnable(ISP_MGR_MIX::EMIX_D3)
        , m_DebugCTL_Disable.bMIX_D3
        , bProfileRequest);

#endif

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
get_P2_AllIndex(const RAWIspCamInfo& rIspCamInfo)
{
    IDXCACHE_STAGE eStage = IDXCACHE_STAGE_P2;

    MUINT16 idxL = 0, idxU = 0;

    if(ISP_MGR_OBC_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_OBC::EOBC_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_OBC, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_OBC, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).isBPCEnable(ISP_MGR_BPC::EBPC_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_BPC_BPC, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_BPC_BPC, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).isCTEnable(ISP_MGR_BPC::EBPC_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_BPC_CT, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_BPC_CT, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_BPC_T::getInstance(m_eSensorDev).isPDCEnable(ISP_MGR_BPC::EBPC_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_BPC_PDC, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_BPC_PDC, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_DM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_DM::EDM_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_DM, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_DM, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_GGM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_GGM::EGGM_D3)
       || ISP_MGR_GGM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_GGM::EGGM_D1)
       || ISP_MGR_IGGM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_IGGM::EIGGM_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_CURRENT, rIspCamInfo, EModule_GMA, eStage);

    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_LDNR_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_LDNR::ELDNR_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_LDNR, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_LDNR, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_EE_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_EE::EEE_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_EE, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_EE, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_YNR_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_YNR::EYNR_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_YNR, eStage);
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_YNR_TBL, eStage);

        if(isIspInterpolation(rIspCamInfo)){
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_YNR, eStage);
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_YNR_TBL, eStage);
        }
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_YNR_T::getInstance(m_eSensorDev).isSkinEnable(ISP_MGR_YNR::EYNR_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_YNR_FD, eStage);
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_YNR_LCE_LINK, eStage);

        if(isIspInterpolation(rIspCamInfo)){
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_YNR_FD, eStage);
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_YNR_LCE_LINK, eStage);
        }
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_CNR_T::getInstance(m_eSensorDev).isCNREnable(ISP_MGR_CNR::ECNR_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_CNR_CNR, eStage);

        if(isIspInterpolation(rIspCamInfo)){
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_CNR_CNR, eStage);
        }
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_CNR_T::getInstance(m_eSensorDev).isCNREnable(ISP_MGR_CNR::ECNR_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_CNR_CCR, eStage);

        if(isIspInterpolation(rIspCamInfo)){
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_CNR_CCR, eStage);
        }
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_CNR_T::getInstance(m_eSensorDev).isCNREnable(ISP_MGR_CNR::ECNR_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_CNR_ABF, eStage);

        if(isIspInterpolation(rIspCamInfo)){
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_CNR_ABF, eStage);
        }
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_MFB_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_MFB::EMFB_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_MFB, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_MFB, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_MIX_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_MIX::EMIX_D3))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_MIX, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_MIX, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_NR3D_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_NR3D::ENR3D_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_NR3D, eStage);

        if(isIspInterpolation(rIspCamInfo))
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_NR3D, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_COLOR_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_COLOR::ECOLOR_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERLV_LOWERCT, rIspCamInfo, EModule_COLOR, eStage);

        if(isIspInterpolation(rIspCamInfo)){
            idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERLV_UPPERCT, rIspCamInfo, EModule_COLOR, eStage);
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERLV_LOWERCT, rIspCamInfo, EModule_COLOR, eStage);
            idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERLV_UPPERCT, rIspCamInfo, EModule_COLOR, eStage);
            idxL = getISPIndex(IDXCACHE_VALTYPE_CURRENT, rIspCamInfo, EModule_COLOR_PARAM, eStage);
        }
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_LCE_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_LCE::ELCE_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_CURRENT, rIspCamInfo, EModule_LCE, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_DCE_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_DCE::EDCE_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_CURRENT, rIspCamInfo, EModule_DCE, eStage);
    }
    else{
        //set cache valid = 0
    }

    if(ISP_MGR_CCM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_CCM::ECCM_D1))
    {
        idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERLV , rIspCamInfo, EModule_CCM, eStage);
        idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERLV , rIspCamInfo, EModule_CCM, eStage);
    }
    else{
        //set cache valid = 0
    }

#if 0

    CAM_LOGD_IF(m_bDebugEnable,"[%s()] Final(%d), ADB_MIX_D3(%d), ProfileRequest(%d)",
        __FUNCTION__
        , rMix.isEnable(ISP_MGR_MIX::EMIX_D3)
        , m_DebugCTL_Disable.bMIX_D3
        , bProfileRequest);

#endif

    return MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC(MUINT8 SubModuleIndex)
{
    MY_LOG_IF(m_bDebugEnable, "%s +", __FUNCTION__);

    if(m_pISPLscResult.rResultBuf != NULL){

        ISP_MGR_LSC_T::getInstance(m_eSensorDev).configTSFS_Size(SubModuleIndex,
                                                                 m_pISPLscResult.u4RawWd, m_pISPLscResult.u4RawHt,
                                                                 m_pISPLscResult.u4TsfsH, m_pISPLscResult.u4TsfsV);
        ISP_MGR_LSC_T::getInstance(m_eSensorDev).putBufAndRatio(SubModuleIndex, *m_pISPLscResult.rResultBuf, m_pISPLscResult.ratio);
        MY_LOG_IF(m_bDebugEnable, "[%s] frame id %d, buf va %p, OnOff %d, ratio %d", __func__,
            m_rIspCamInfo.u4Id, m_pISPLscResult.rResultBuf, m_pISPLscResult.fgOnOff, m_pISPLscResult.ratio);

        m_rIspCamInfo.rSl2Info.i4CenterX  = m_pISPLscResult.i4CenterX;
        m_rIspCamInfo.rSl2Info.i4CenterY  = m_pISPLscResult.i4CenterY;
        m_rIspCamInfo.rSl2Info.i4R0       = m_pISPLscResult.i4R0;
        m_rIspCamInfo.rSl2Info.i4R1       = m_pISPLscResult.i4R1;
        m_rIspCamInfo.rSl2Info.i4R2       = m_pISPLscResult.i4R2;
        m_rIspCamInfo.rSl2Info.i4Gain0    = m_pISPLscResult.i4Gain0;
        m_rIspCamInfo.rSl2Info.i4Gain1    = m_pISPLscResult.i4Gain1;
        m_rIspCamInfo.rSl2Info.i4Gain2    = m_pISPLscResult.i4Gain2;
        m_rIspCamInfo.rSl2Info.i4Gain3    = m_pISPLscResult.i4Gain3;
        m_rIspCamInfo.rSl2Info.i4Gain4    = m_pISPLscResult.i4Gain4;

        MY_LOG_IF(m_bDebugEnable, "[%s] SL2 Gain0 %d, Gain1 %d, Gain2 %d, Gain3 %d, Gain4 %d",
            __func__,
            m_pISPLscResult.i4Gain0, m_pISPLscResult.i4Gain1, m_pISPLscResult.i4Gain2,
            m_pISPLscResult.i4Gain3, m_pISPLscResult.i4Gain4);

        m_rIspCamInfo.eIdx_Shading_CCT     = (NSIspTuning::EIndex_Shading_CCT_T)m_pISPLscResult.eIdx_Shading_CCT;
    } else {

        MY_ERR("[%s] m_pISPLscResult->rResultBuf is null", __func__);
    }

    m_rIspCamInfo.rSl2Info.bLSC_R1_En = ISP_MGR_LSC_T::getInstance(m_eSensorDev).isEnable(0);

    MY_LOG_IF(m_bDebugEnable, "%s -", __FUNCTION__);

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_OBC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_OBC_T& rObc = ISP_MGR_OBC_T::getInstance(m_eSensorDev);
    ISP_MGR_MOBC_T& rMobc = ISP_MGR_MOBC_T::getInstance(m_eSensorDev);

    MBOOL bMOBC_Enable = MFALSE;
    if(SubModuleIndex < ISP_MGR_MOBC::ESubModule_NUM){
        //MOBC only exist in P1
        bMOBC_Enable = rMobc.isEnable(SubModuleIndex);
    }

    if(rObc.isEnable(SubModuleIndex)|| bMOBC_Enable){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_OBC, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_OBC_T obc = m_rIspParam.ISPRegs.OBC[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_OBC, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_OBC, eStage, IDXCACHE_VALTYPE_UPPERISO);
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P1RealISOValue;

            if(eStage = IDXCACHE_STAGE_P2){
                u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;
            }

            AAA_TRACE_ALG(SmoothOBC);
            Smooth_OBC(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.OBC[idxU], // OBC settings for upper ISO
                    m_rIspParam.ISPRegs.OBC[idxL], // OBC settings for lower ISO
                    obc);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_OBC(SubModuleIndex, rIspCamInfo, obc); }

        this->setPureOBCInfo(&obc);

        if(rObc.isEnable(SubModuleIndex)){ rObc.put(SubModuleIndex, obc); }
        else if(bMOBC_Enable){ rMobc.put(SubModuleIndex, obc); }
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_BPC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if(rBpc.isBPCEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_BPC_BPC, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_BPC_BPC_T bpc = m_rIspParam.ISPRegs.BPC_BPC[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_BPC_BPC, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_BPC_BPC, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P1RealISOValue;

            if(eStage = IDXCACHE_STAGE_P2){
                u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;
            }

            AAA_TRACE_ALG(SmoothBPC);
            Smooth_BPC_BPC(u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.BPC_BPC[idxU], // BPC settings for upper ISO
                    m_rIspParam.ISPRegs.BPC_BPC[idxL], // BPC settings for lower ISO
                    bpc);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_BPC(SubModuleIndex, rIspCamInfo, bpc); }

        rBpc.put(SubModuleIndex, bpc);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_CT(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if(rBpc.isCTEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_BPC_CT, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_BPC_CT_T ct = m_rIspParam.ISPRegs.BPC_CT[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_BPC_CT, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_BPC_CT, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P1RealISOValue;

            if(eStage = IDXCACHE_STAGE_P2){
                u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;
            }

            AAA_TRACE_ALG(SmoothCT);
            Smooth_BPC_CT(u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.BPC_CT[idxU],
                    m_rIspParam.ISPRegs.BPC_CT[idxL],
                    ct);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_CT(SubModuleIndex, rIspCamInfo, ct); }

        rBpc.put(SubModuleIndex, ct);
    }
    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_PDC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_BPC_T& rBpc = ISP_MGR_BPC_T::getInstance(m_eSensorDev);

    if(rBpc.isPDCEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_BPC_PDC, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_BPC_PDC_T pdc = m_rIspParam.ISPRegs.BPC_PDC[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_BPC_PDC, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_BPC_PDC, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P1RealISOValue;

            if(eStage = IDXCACHE_STAGE_P2){
                u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;
            }

            AAA_TRACE_ALG(SmoothPDC);
            Smooth_BPC_PDC(u4RealISOValue,  // Real ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.BPC_PDC[idxU],
                    m_rIspParam.ISPRegs.BPC_PDC[idxL],
                    pdc);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_PDC(SubModuleIndex, rIspCamInfo, pdc); }

        rBpc.put(SubModuleIndex, pdc);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_DM(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_DM_T& rDm = ISP_MGR_DM_T::getInstance(m_eSensorDev);

    if(rDm.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_DM, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_DM_T dm = m_rIspParam.ISPRegs.DM[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_DM, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_DM, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P1RealISOValue;

            if(eStage = IDXCACHE_STAGE_P2){
                u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;
            }

            AAA_TRACE_ALG(SmoothDM);
            Smooth_DM(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.DM[idxU],
                    m_rIspParam.ISPRegs.DM[idxL],
                    dm);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_DM(SubModuleIndex, rIspCamInfo, dm); }

        rDm.put(SubModuleIndex, dm);
    }

    return  MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_GGM(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_GGM_T& rGgm = ISP_MGR_GGM_T::getInstance(m_eSensorDev);
    ISP_MGR_IGGM_T& rIggm = ISP_MGR_IGGM_T::getInstance(m_eSensorDev);

    MBOOL bMainGGM_Enable = MFALSE;
    if(SubModuleIndex == ISP_MGR_GGM::EGGM_D3){
        //Even if GGM_D3 is off, we have to get GGM_D1 & IGGM_D1 by m_pGmaMgr
        bMainGGM_Enable = rGgm.isEnable(ISP_MGR_GGM::EGGM_D1)
                          || rGgm.isEnable(ISP_MGR_GGM::EGGM_D3)
                          || rIggm.isEnable(ISP_MGR_IGGM::EIGGM_D1);
    }

    if(rGgm.isEnable(SubModuleIndex) || bMainGGM_Enable){

        ISP_NVRAM_GGM_T ggm = {0};
        MUINT16 idx = m_rIdxCache.getCacheValue(EModule_GMA, eStage, IDXCACHE_VALTYPE_CURRENT);

        switch (SubModuleIndex)
        {
            case ISP_MGR_GGM::EGGM_R1:
            case ISP_MGR_GGM::EGGM_D2:
                //Stereo
                ggm = m_rIspParam.ISPToneMap.GMA[idx].GGM_Reg[0];
                break;
            case ISP_MGR_GGM::EGGM_R2:
                //Face
                if(rIspCamInfo.rFdInfo.FaceGGM_Idx == 0){ m_pGmaMgr->getLastGGM(ggm);}
                else{ ggm = rIspCamInfo.rFdInfo.FaceGGM;}
                break;
            case ISP_MGR_GGM::EGGM_D3:
            {
                if(rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE){
                    ggm = m_pGgmMgr->getGGM();
                }
                else{
                    ISP_NVRAM_GGM_T ggm_d1 = {0};
                    ISP_NVRAM_IGGM_T iggm_d1 = {0};
                    AAA_TRACE_ALG(calculateGGM);
                    m_pGmaMgr->calculateGGM(rIspCamInfo, idx, &ggm_d1, &iggm_d1, &ggm, &m_GmaExifInfo,
                                            m_bSlave, m_ISP_Sync_Info);
                    AAA_TRACE_END_ALG;
                    m_pGgmMgr->updateGGM(ggm);
                    if(rGgm.isEnable(ISP_MGR_GGM::EGGM_D1)){ rGgm.put(ISP_MGR_GGM::EGGM_D1, ggm_d1);}
                    if(rIggm.isEnable(ISP_MGR_IGGM::EIGGM_D1)){ rIggm.put(ISP_MGR_IGGM::EIGGM_D1, iggm_d1);}
                }
            }
                break;
            default:
                CAM_LOGE("Apply Error Submodule Index: %d", SubModuleIndex);
                return  MFALSE;
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_GGM(SubModuleIndex, rIspCamInfo, ggm); }

        rGgm.put(SubModuleIndex, ggm);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_FLC(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_FLC_T& rFlc = ISP_MGR_FLC_T::getInstance(m_eSensorDev);

    if(rFlc.isEnable(SubModuleIndex)){

        ISP_NVRAM_FLC_T flc = {0};
        m_pDceMgr->getLastFLC(flc);

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_FLC(SubModuleIndex, rIspCamInfo, flc); }

        rFlc.put(SubModuleIndex, flc);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_LCES(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_LCES_T& rLces = ISP_MGR_LCES_T::getInstance(m_eSensorDev);
    ISP_MGR_CCM_T& rCcm = ISP_MGR_CCM_T::getInstance(m_eSensorDev);

    if(rLces.isEnable(SubModuleIndex)){

        ISP_NVRAM_FLC_T flc = {0};
        m_pDceMgr->getLastFLC(flc);
        rLces.put(SubModuleIndex, flc);
        rLces.put(SubModuleIndex, rIspCamInfo.rMtkCCM);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_LDNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_LDNR_T& rLdnr = ISP_MGR_LDNR_T::getInstance(m_eSensorDev);

    if(rLdnr.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_LDNR, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_LDNR_T ldnr = m_rIspParam.ISPRegs.LDNR[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_LDNR, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_LDNR, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothLDNR);
            Smooth_LDNR(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.LDNR[idxU],
                    m_rIspParam.ISPRegs.LDNR[idxL],
                    ldnr);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_LDNR(SubModuleIndex, rIspCamInfo, ldnr); }

        rLdnr.put(SubModuleIndex, ldnr);
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_G2CX(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo)
{
    ISP_MGR_G2CX_T& rG2cx = ISP_MGR_G2CX_T::getInstance(m_eSensorDev);

    if(rG2cx.isEnable(SubModuleIndex)){

        ISP_NVRAM_G2CX_T g2cx = {0};

        m_pIspTuningCustom->userSetting_HSBC(rIspCamInfo.rIspUsrSelectLevel, g2cx);

        rG2cx.put(SubModuleIndex, g2cx);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_EE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_EE_T& rEe = ISP_MGR_EE_T::getInstance(m_eSensorDev);

    if(rEe.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_EE, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_EE_T ee = m_rIspParam.ISPRegs.EE[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_EE, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_EE, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothEE);
            Smooth_EE(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.EE[idxU],
                    m_rIspParam.ISPRegs.EE[idxL],
                    ee);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_EE(SubModuleIndex, rIspCamInfo, ee); }

#if 0
        if (rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
        {
            m_pIspTuningCustom->userSetting_EE(rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);
        }
#endif

        rEe.put(SubModuleIndex, ee);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_YNRS(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_YNRS_T& rYnrs = ISP_MGR_YNRS_T::getInstance(m_eSensorDev);

    if(rYnrs.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_YNRS, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_YNRS_T ynrs = m_rIspParam.ISPRegs.YNRS[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_YNRS, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_YNRS, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P1RealISOValue;

            AAA_TRACE_ALG(SmoothYNRS);
            Smooth_YNRS(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.YNRS[idxU],
                    m_rIspParam.ISPRegs.YNRS[idxL],
                    ynrs);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_YNRS(SubModuleIndex, rIspCamInfo, ynrs); }

        rYnrs.put(SubModuleIndex, ynrs);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_YNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_YNR_T& rYnr = ISP_MGR_YNR_T::getInstance(m_eSensorDev);

    if(rYnr.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_YNR, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_YNR_T ynr = m_rIspParam.ISPRegs.YNR[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_YNR, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_YNR, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothYNR);
            Smooth_YNR(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.YNR[idxU],
                    m_rIspParam.ISPRegs.YNR[idxL],
                    ynr);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_YNR(SubModuleIndex, rIspCamInfo, ynr); }

        rYnr.put(SubModuleIndex, ynr);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_YNR_TBL(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_YNRS_T& rYnrs = ISP_MGR_YNRS_T::getInstance(m_eSensorDev);
    ISP_MGR_YNR_T& rYnr = ISP_MGR_YNR_T::getInstance(m_eSensorDev);

    MBOOL bYNR_TBL_Enable = (eStage == IDXCACHE_STAGE_P1) ? rYnrs.isEnable(SubModuleIndex) : rYnr.isEnable(SubModuleIndex);

    if(bYNR_TBL_Enable){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_YNR_TBL, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_YNR_LUT_T ynr_tbl = m_rIspParam.ISPRegs.YNR_TBL[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_YNR_TBL, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_YNR_TBL, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P1RealISOValue;

            if(eStage = IDXCACHE_STAGE_P2){
                u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;
            }

            AAA_TRACE_ALG(SmoothYNRTBL);
            Smooth_YNR_TBL(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.YNR_TBL[idxU],
                    m_rIspParam.ISPRegs.YNR_TBL[idxL],
                    ynr_tbl);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ /*m_pIspTuningCustom->refine_YNRS(m_rIspCamInfo, m_IspNvramMgr, ynrs); */ }

        if(eStage == IDXCACHE_STAGE_P1){ rYnrs.put(SubModuleIndex, ynr_tbl);}
        if(eStage == IDXCACHE_STAGE_P2){ rYnr.put(SubModuleIndex, ynr_tbl);}

        // Dump P2 ANR_TBL intrp result
        if(eStage == IDXCACHE_STAGE_P2){

            MINT32 captureDump = property_get_int32("vendor.isp.ynr_tbl.dump", 0);
            MINT32 previewDump = property_get_int32("vendor.isp.ynr_tbl.dump.preview", 0);
            MBOOL bDumpTbl = MFALSE;

            if ((!rIspCamInfo.fgRPGEnable && captureDump && (rIspCamInfo.bEnableNDD || (rIspCamInfo.i4P2InImgFmt != EYuv2Yuv)))
                || previewDump)
                bDumpTbl = MTRUE;

            if (bDumpTbl){
                char filename[512] = "";
                genFileName_YNR_INT_TBL(filename, sizeof(filename), &m_hint_p2);
                android::sp<IFileCache> fidYnrIntDump;
                fidYnrIntDump = IFileCache::open(filename);
                if (fidYnrIntDump->write(&ynr_tbl, sizeof(ISP_NVRAM_YNR_LUT_T)) != sizeof(ISP_NVRAM_YNR_LUT_T))
                    CAM_LOGW("[%s] fail dump to %s", __FUNCTION__, filename);
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
getNvram_PerFrame_YNR_FACE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_YNR_T& rYnr = ISP_MGR_YNR_T::getInstance(m_eSensorDev);

    if(rYnr.isSkinEnable(SubModuleIndex)){

        //YNR_FD SW Param (m_ynr_fd)
        {
            MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_YNR_FD, eStage, IDXCACHE_VALTYPE_LOWERISO);
            m_ynr_fd = m_rIspParam.ISPRegs.YNR_FD[idxL];

            //Interpolation YNR_FD SW Param
            if (isIspInterpolation(rIspCamInfo))
            {
                MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_YNR_FD, eStage, IDXCACHE_VALTYPE_UPPERISO);
                EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_YNR_FD, eStage, IDXCACHE_VALTYPE_UPPERISO);;
                MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

                AAA_TRACE_ALG(Smooth_YNRFD);
                Smooth_YNR_FD(u4RealISOValue,
                        m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                        m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                        m_rIspParam.ISPRegs.YNR_FD[idxU],
                        m_rIspParam.ISPRegs.YNR_FD[idxL],
                        m_ynr_fd);  // Output
                AAA_TRACE_END_ALG;

                CAM_LOGD_IF(m_bDebugEnable,"%s(Smooth_YNR_FD): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                        __FUNCTION__, SubModuleIndex, idxU, idxL);
            }
        }

        ISP_MGR_GGM_T& rGgm = ISP_MGR_GGM_T::getInstance(m_eSensorDev);
        ISP_NVRAM_YNR_LUT_T face_ynr_tbl = {0};

        IImageBuffer* pAlphaMap = (IImageBuffer*)rYnr.getAlphaMapForWrite(SubModuleIndex, rIspCamInfo.u4Id);
        if (pAlphaMap == nullptr){
            CAM_LOGE("[%s] Fail to get alpha map for write!", __FUNCTION__);
            return MFALSE;
        }

        char *p_face_map = (char *) pAlphaMap->getBufVA(0);
        //FaceYNR (ISP_NVRAM_YNR_SKIN_T, ANR_TBL, AlphaMap)
        {
            if ((pAlphaMap != NULL) && (p_face_map != NULL)){

                MFLOAT fRzRto = (MFLOAT)rIspCamInfo.rCropRzInfo.sRRZout.h/(MFLOAT)rIspCamInfo.rCropRzInfo.sRRZout.w;
                m_FaceIsize.w = 320;
                m_FaceIsize.h = (MINT32)(320.0f * fRzRto);
                pAlphaMap->setExtParam(m_FaceIsize, 0);
                CAM_LOGD_IF(m_bDebugEnable,"%s(): m_FaceIsize.w = %d, m_FaceIsize.h = %d, rto(%f), rrzw(%d), rrzh(%d)\n",
                   __FUNCTION__, m_FaceIsize.w, m_FaceIsize.h, fRzRto, rIspCamInfo.rCropRzInfo.sRRZout.w, rIspCamInfo.rCropRzInfo.sRRZout.h);

                ISP_NVRAM_YNR_T ynr = {0};
                rYnr.get(SubModuleIndex, ynr);
                ISP_NVRAM_YNR_SKIN_T ynr_skin = {0};
                ynr_skin.skin_con.val = ynr.skin_con.val;
                rYnr.get(SubModuleIndex, face_ynr_tbl);

                ISP_NVRAM_GGM_T ggm_d1 = {0};
                rGgm.get(ISP_MGR_GGM::EGGM_D1, ggm_d1);

                AAA_TRACE_ALG(FaceYNR);
                FaceYNR(rIspCamInfo, rGgm.getEndVar(ISP_MGR_GGM::EGGM_R2), &ggm_d1, rGgm.getEndVar(ISP_MGR_GGM::EGGM_D1), m_ynr_fd, ynr_skin, face_ynr_tbl, pAlphaMap);
                AAA_TRACE_END_ALG;

                rYnr.put(SubModuleIndex, ynr_skin);
                rYnr.put(SubModuleIndex, face_ynr_tbl);
                //Dump Table

            }
            else{
                rYnr.setSkinEnable(SubModuleIndex, MFALSE);
				CAM_LOGE("[%s]  pAlphaMap(%p), p_face_map(%p) ",__FUNCTION__, pAlphaMap, p_face_map);
            }

            //dump table
            if(rYnr.isSkinEnable(SubModuleIndex)){
                MINT32 captureDump = property_get_int32("vendor.isp.ynr_tbl.dump", 0);
                MINT32 previewDump = property_get_int32("vendor.isp.ynr_tbl.dump.preview", 0);
                MBOOL bDumpTbl = MFALSE;

                if ((!rIspCamInfo.fgRPGEnable && captureDump && (rIspCamInfo.bEnableNDD || (rIspCamInfo.i4P2InImgFmt != EYuv2Yuv)))
                    || previewDump)
                    bDumpTbl = MTRUE;

                if (bDumpTbl)
                {
                    char filename[512] = "";
                    genFileName_YNR_ALPHA_MAP(filename, sizeof(filename), &m_hint_p2);
                    if (pAlphaMap != NULL){
                        pAlphaMap->saveToFile(filename);
                    }

                    genFileName_YNR_FD_TBL(filename, sizeof(filename), &m_hint_p2);
                    android::sp<IFileCache> fidYnrFdDump;
                    fidYnrFdDump = IFileCache::open(filename);
                    CAM_LOGW("[%s] %s", __FUNCTION__, filename);
                    if (fidYnrFdDump->write(&face_ynr_tbl, sizeof(ISP_NVRAM_YNR_LUT_T)) != sizeof(ISP_NVRAM_YNR_LUT_T))
                        CAM_LOGW("[%s] fail dump to %s", __FUNCTION__, filename);
                }
            }
        }

        //YNR_LCE_SW SW Param (m_sw_ynr)
        {
            MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_YNR_LCE_LINK, eStage, IDXCACHE_VALTYPE_LOWERISO);
            m_sw_ynr = m_rIspParam.ISPRegs.YNR_LCE_LINK[idxL];

            //Interpolation YNR_LCE_SW SW Param
            if (isIspInterpolation(rIspCamInfo))
            {
                MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_YNR_LCE_LINK, eStage, IDXCACHE_VALTYPE_UPPERISO);
                EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_YNR_LCE_LINK, eStage, IDXCACHE_VALTYPE_UPPERISO);;
                MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

                AAA_TRACE_ALG(SmoothYNRLCESW);
                SmoothYNR_LCE_SW(u4RealISOValue,
                        m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                        m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                        m_rIspParam.ISPRegs.YNR_LCE_LINK[idxU],
                        m_rIspParam.ISPRegs.YNR_LCE_LINK[idxL],
                        m_sw_ynr);  // Output
                AAA_TRACE_END_ALG;

                CAM_LOGD_IF(m_bDebugEnable,"%s(SmoothYNR_LCE_SW): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                        __FUNCTION__, SubModuleIndex, idxU, idxL);
            }
        }

        //Calculate_YNR_LCE (ISP_NVRAM_YNR_LCE_OUT_T, ANR_TBL)
        {
            ISP_NVRAM_GGM_T ggm_d3 = {0};
            rGgm.get(ISP_MGR_GGM::EGGM_D3, ggm_d3);
            ISP_MGR_LCE_T& rLce = ISP_MGR_LCE_T::getInstance(m_eSensorDev);
            ISP_NVRAM_LCE_T lce = {0};
            rLce.get(ISP_MGR_LCE::ELCE_D1, lce);
            ISP_NVRAM_YNR_LCE_OUT_T Ynr_LceResult = {0};
            m_SW_YNR_LTM_Ver =100;
            ISP_NVRAM_YNR_LUT_T ynr_tbl_final = {0};
            ISP_MGR_YNR_T::getInstance(m_eSensorDev).get(SubModuleIndex, ynr_tbl_final);

            AAA_TRACE_ALG(CalculateYNRLCE);
            Calculate_YNR_LCE(
                rLce.getGlobleTone(ISP_MGR_LCE::ELCE_D1),
                rLce.isEnable(ISP_MGR_LCE::ELCE_D1),
                ggm_d3, rGgm.getEndVar(ISP_MGR_GGM::EGGM_D3),
                lce,
                ynr_tbl_final,
                m_sw_ynr,
                m_SW_YNR_LTM_Ver,
                Ynr_LceResult);
            AAA_TRACE_END_ALG;

            rYnr.put(ISP_MGR_YNR::EYNR_D1, Ynr_LceResult);
            rYnr.put(ISP_MGR_YNR::EYNR_D1, ynr_tbl_final);
        }
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_CNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);

    if(rCnr.isCNREnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_CNR_CNR, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_CNR_CNR_T cnr = m_rIspParam.ISPRegs.CNR_CNR[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_CNR_CNR, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_CNR_CNR, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothCNR);
            Smooth_CNR_CNR(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.CNR_CNR[idxU],
                    m_rIspParam.ISPRegs.CNR_CNR[idxL],
                    cnr);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_CNR(SubModuleIndex, rIspCamInfo, cnr); }

        rCnr.put(SubModuleIndex, cnr);
    }

    return  MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_CCR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);

    if(rCnr.isCCREnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_CNR_CCR, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_CNR_CCR_T ccr = m_rIspParam.ISPRegs.CNR_CCR[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_CNR_CCR, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_CNR_CCR, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothCCR);
            Smooth_CNR_CCR(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.CNR_CCR[idxU],
                    m_rIspParam.ISPRegs.CNR_CCR[idxL],
                    ccr);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_CCR(SubModuleIndex, rIspCamInfo, ccr); }

        rCnr.put(SubModuleIndex, ccr);
    }

    return  MTRUE;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_ABF(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_CNR_T& rCnr = ISP_MGR_CNR_T::getInstance(m_eSensorDev);

    if(rCnr.isABFEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_CNR_ABF, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_CNR_ABF_T abf = m_rIspParam.ISPRegs.CNR_ABF[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_CNR_ABF, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_CNR_ABF, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothABF);
            Smooth_CNR_ABF(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.CNR_ABF[idxU],
                    m_rIspParam.ISPRegs.CNR_ABF[idxL],
                    abf);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_ABF(SubModuleIndex, rIspCamInfo, abf); }

        rCnr.put(SubModuleIndex, abf);
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_COLOR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_COLOR_T& rColor = ISP_MGR_COLOR_T::getInstance(m_eSensorDev);

    if(rColor.isEnable(SubModuleIndex)){

        MUINT16 idxLL = m_rIdxCache.getCacheValue(EModule_COLOR, eStage, IDXCACHE_VALTYPE_LOWERLV_LOWERCT);
        ISP_NVRAM_COLOR_T color = m_rIspParam.ISPColorTbl.COLOR[idxLL].COLOR_TBL;

        if( m_rIspParam.ISPColorTbl.COMM.COLOR.COLOR_Method && isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxLU = m_rIdxCache.getCacheValue(EModule_COLOR, eStage, IDXCACHE_VALTYPE_LOWERLV_UPPERCT);
            MUINT16 idxUL = m_rIdxCache.getCacheValue(EModule_COLOR, eStage, IDXCACHE_VALTYPE_UPPERLV_LOWERCT);
            MUINT16 idxUU = m_rIdxCache.getCacheValue(EModule_COLOR, eStage, IDXCACHE_VALTYPE_UPPERLV_UPPERCT);
            MUINT16 idxColor = m_rIdxCache.getCacheValue(EModule_COLOR_PARAM, eStage, IDXCACHE_VALTYPE_UPPERLV_UPPERCT);

            MBOOL bIsCapture = m_pIspTuningCustom->is_to_invoke_capture_in_color(rIspCamInfo);
            ColorInfo rColorInfo = {0};

            rColorInfo.RealLV = rIspCamInfo.rAEInfo.i4RealLightValue_x10;
            rColorInfo.AWB_NoPrefGain[0] = rIspCamInfo.rAWBInfo.rAwbGainNoPref.i4R;
            rColorInfo.AWB_NoPrefGain[1] = rIspCamInfo.rAWBInfo.rAwbGainNoPref.i4G;
            rColorInfo.AWB_NoPrefGain[2] = rIspCamInfo.rAWBInfo.rAwbGainNoPref.i4B;
            rColorInfo.NoPrefCCT = rIspCamInfo.rAWBInfo.i4CCT;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): i4RealLightValue_x10 = %d, i4CCT = %d, u2LowerLv_LowerCt = %d, u2LowerLv_UpperCt = %d, u2UpperLv_LowerCt = %d, u2UpperLv_UpperCt = %d, u2ParamIdx = %d\n",
                       __FUNCTION__, rIspCamInfo.rAEInfo.i4RealLightValue_x10, rIspCamInfo.rAWBInfo.i4CCT,
                       idxLL, idxLU, idxUL, idxUU, idxColor);

            AAA_TRACE_ALG(SmoothCOLOR);

            SmoothCOLOR( bIsCapture,
                         m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_U],
                         m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_L],
                         m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
                         m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L],
                         rColorInfo,
                         rIspCamInfo.rFdInfo,
                         m_rIspParam.ISPColorTbl.COLOR[idxLL],
                         m_rIspParam.ISPColorTbl.COLOR[idxLU],
                         m_rIspParam.ISPColorTbl.COLOR[idxUL],
                         m_rIspParam.ISPColorTbl.COLOR[idxUU],
                         m_rIspParam.ISPColorTbl.COLOR_PARAM[idxColor],
                         color,
                         m_ISP_ALG_VER.sCOLOR,
                         m_bSmoothColor_FirstTimeBoot,
                         m_SmoothColor_Prv);

            AAA_TRACE_END_ALG;
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_COLOR(SubModuleIndex, rIspCamInfo, color); }

        rColor.put(SubModuleIndex, color);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_MFB(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_MFB_T& rMfb = ISP_MGR_MFB_T::getInstance(m_eSensorDev);

    if(rMfb.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_MFB, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_MFB_T mfb = m_rIspParam.ISPRegs.MFB[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_MFB, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_MFB, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothMFB);
            Smooth_MFB(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.MFB[idxU],
                    m_rIspParam.ISPRegs.MFB[idxL],
                    mfb);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_MFB(SubModuleIndex, rIspCamInfo, mfb); }

        rMfb.put(SubModuleIndex, mfb);
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_MIX(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_MIX_T& rMix = ISP_MGR_MIX_T::getInstance(m_eSensorDev);

    if(rMix.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_MIX, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_MIX_T mix = m_rIspParam.ISPRegs.MIX[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_MIX, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_MIX, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothMIX);
            Smooth_MIX(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.MIX[idxU],
                    m_rIspParam.ISPRegs.MIX[idxL],
                    mix);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_MIX(SubModuleIndex, rIspCamInfo, mix); }

        rMix.put(SubModuleIndex, mix);
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_NR3D(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_NR3D_T& rNr3d = ISP_MGR_NR3D_T::getInstance(m_eSensorDev);

    if(rNr3d.isEnable(SubModuleIndex)){

        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_NR3D, eStage, IDXCACHE_VALTYPE_LOWERISO);
        ISP_NVRAM_NR3D_T nr3d = m_rIspParam.ISPRegs.NR3D[idxL];

        //Interpolation ISP module
        if (isIspInterpolation(rIspCamInfo))
        {
            MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_NR3D, eStage, IDXCACHE_VALTYPE_UPPERISO);
            EISO_GROUP_T ISO_GroupIDX = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_NR3D, eStage, IDXCACHE_VALTYPE_UPPERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            AAA_TRACE_ALG(SmoothNR3D);
            Smooth_NR3D(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_U[ISO_GroupIDX]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[ISO_GroupIDX].IDX_Partition[rIspCamInfo.eIdx_ISO_L[ISO_GroupIDX]], // Lower ISO
                    m_rIspParam.ISPRegs.NR3D[idxU],
                    m_rIspParam.ISPRegs.NR3D[idxL],
                    nr3d);  // Output
            AAA_TRACE_END_ALG;

            CAM_LOGD_IF(m_bDebugEnable,"%s(): SubModuleIndex(%d), idxU(%d), idxL(%d)\n",
                    __FUNCTION__, SubModuleIndex, idxU, idxL);
        }

        if(isDynamicTuning()){ m_pIspTuningCustom->refine_NR3D(SubModuleIndex, rIspCamInfo, nr3d); }

        rNr3d.put(SubModuleIndex, nr3d);
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_LPCNR(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    //Note: LPCNR need an independent Pass2 round
    //get_P2_AllIndex doesn't support LPCNR Index

    ISP_MGR_LPCNR_T& rLpcnr = ISP_MGR_LPCNR_T::getInstance(m_eSensorDev);

    if(rLpcnr.isEnable(SubModuleIndex)){
        MUINT16 idxL = getISPIndex(IDXCACHE_VALTYPE_LOWERISO , rIspCamInfo, EModule_LPCNR, eStage);
        ISP_NVRAM_LPCNR_T lpcnr = m_rIspParam.ISPRegs.LPCNR[idxL];

        if (isIspInterpolation(rIspCamInfo))
        {
            EISO_GROUP_T SubModuleIndexGroup = (EISO_GROUP_T)m_rIdxCache.getIsoGroupValue(EModule_LPCNR, eStage, IDXCACHE_VALTYPE_LOWERISO);;
            MUINT32 u4RealISOValue = rIspCamInfo.rAEInfo.u4P2RealISOValue;

            MUINT16 idxU = getISPIndex(IDXCACHE_VALTYPE_UPPERISO , rIspCamInfo, EModule_LPCNR, eStage);

            CAM_LOGD_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.rAEInfo.u4RealISOValue = %d, u2UpperIso = %d, u2LowerIso = %d\n",
                        __FUNCTION__, u4RealISOValue, idxU, idxL);

            AAA_TRACE_ALG(Smooth_LPCNR);
            Smooth_LPCNR(u4RealISOValue,
                    m_rIspParam.ISPRegs.Iso_Env[SubModuleIndexGroup].IDX_Partition[rIspCamInfo.eIdx_ISO_U[SubModuleIndexGroup]], // Upper ISO
                    m_rIspParam.ISPRegs.Iso_Env[SubModuleIndexGroup].IDX_Partition[rIspCamInfo.eIdx_ISO_L[SubModuleIndexGroup]], // Lower ISO
                    m_rIspParam.ISPRegs.LPCNR[idxU],
                    m_rIspParam.ISPRegs.LPCNR[idxL],
                    lpcnr);  // Output
            AAA_TRACE_END_ALG;
        }

        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            //m_pIspTuningCustom->refine_OBC(m_rIspCamInfo, m_IspNvramMgr, obc);
        }

        rLpcnr.put(SubModuleIndex, lpcnr);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_CCM_P1(MUINT8 SubModuleIndex, RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_CCM_T& rCcm = ISP_MGR_CCM_T::getInstance(m_eSensorDev);

    ISP_NVRAM_CCM_T ccm = {0};

    if(m_rIspCamInfo.eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX){
        ccm = m_pCcmMgr->getCCM();
    }
    else if (isDynamicCCM() &&(!rIspCamInfo.rAWBInfo.bAWBLock))
    {
        MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_CCM, eStage, IDXCACHE_VALTYPE_LOWERLV);
        MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_CCM, eStage, IDXCACHE_VALTYPE_UPPERLV);

        CAM_LOGD_IF(m_bDebugEnable,"%s(%d): i4RealLightValue_x10 = %d, u2LowerLv = %d, u2UpperLv = %d\n",
                   __FUNCTION__, SubModuleIndex, rIspCamInfo.rAEInfo.i4RealLightValue_x10, idxL, idxU);

        AAA_TRACE_ALG(calculateCCM);
        m_pCcmMgr->calculateCCM(ccm, m_ISP_ALG_VER.sCCM, rIspCamInfo,
            m_rIspParam.ISPColorTbl.CCM[idxU],
            m_rIspParam.ISPColorTbl.CCM[idxL],
            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L]);
        AAA_TRACE_END_ALG;

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            //m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, rCCM);
        }
    }
    else {
        ccm = m_pCcmMgr->getPrvCCM();//take output directly
    }

    m_pCcmMgr->updateCCM(ccm);//for auto ccm result
    rIspCamInfo.rMtkCCM = ccm;

    if(rCcm.isEnable(SubModuleIndex)){
        rCcm.put(SubModuleIndex, ccm);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_CCM_P2(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_CCM_T& rCcm = ISP_MGR_CCM_T::getInstance(m_eSensorDev);

    if(rCcm.isEnable(SubModuleIndex)){

        ISP_NVRAM_CCM_T ccm = rIspCamInfo.rMtkCCM;

        if (((rIspCamInfo.eColorCorrectionMode != MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) && (!rIspCamInfo.rAWBInfo.bAWBLock)) ||
             (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_toW)  ||
             (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW)  ||
             (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Video_toW) ||
             (rIspCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Depth_toW))
        {
            if (isDynamicCCM())
            {
                MUINT16 idxL = m_rIdxCache.getCacheValue(EModule_CCM, eStage, IDXCACHE_VALTYPE_LOWERLV);
                MUINT16 idxU = m_rIdxCache.getCacheValue(EModule_CCM, eStage, IDXCACHE_VALTYPE_UPPERLV);

                CAM_LOGD_IF(m_bDebugEnable,"%s(%d): i4RealLightValue_x10 = %d, u2LowerLv = %d, u2UpperLv = %d\n",
                           __FUNCTION__, SubModuleIndex, rIspCamInfo.rAEInfo.i4RealLightValue_x10, idxL, idxU);

                AAA_TRACE_ALG(calculateCCM);
                    m_pCcmMgr->calculateCCM(ccm, m_ISP_ALG_VER.sCCM, rIspCamInfo,
                                            m_rIspParam.ISPColorTbl.CCM[idxU],
                                            m_rIspParam.ISPColorTbl.CCM[idxL],
                                            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
                                            m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L]);
                AAA_TRACE_END_ALG;

                // Invoke callback for customers to modify.
                if  ( isDynamicTuning() ){ m_pIspTuningCustom->refine_CCM(SubModuleIndex, rIspCamInfo, ccm); }
            }
        }
        rCcm.put(SubModuleIndex, ccm);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_LCE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    CAM_LOGD_IF(m_bDebugEnable,"[%s] FrmId(%d), LCSOId(%d), Profile(%d), Buffer(%p), W(%d), H(%d)",
    __func__,
    rIspCamInfo.u4Id,
    rIspCamInfo.rLCS_Info.i4FrmId,
    rIspCamInfo.rMapping_Info.eIspProfile,
    m_pLCESBuffer,
    rIspCamInfo.rLCS_Info.u4OutWidth,
    rIspCamInfo.rLCS_Info.u4OutHeight);

    ISP_MGR_LCE_T& rLce = ISP_MGR_LCE_T::getInstance(m_eSensorDev);

    if(rLce.isEnable(SubModuleIndex) && (m_pLCESBuffer!=NULL)){

        MSize tempLCEIsize(rIspCamInfo.rLCS_Info.u4OutWidth, rIspCamInfo.rLCS_Info.u4OutHeight);
        m_pLCESBuffer->setExtParam(tempLCEIsize, 0);

        ISP_NVRAM_LCE_T lce = {0};
        ISP_NVRAM_WB_T wb;
        wb.gain_1.bits.WB_GAIN_B = rIspCamInfo.rAWBInfo.rPGN.i4B;
        wb.gain_1.bits.WB_GAIN_GB = rIspCamInfo.rAWBInfo.rPGN.i4G;
        wb.gain_2.bits.WB_GAIN_GR = rIspCamInfo.rAWBInfo.rPGN.i4G;
        wb.gain_2.bits.WB_GAIN_R = rIspCamInfo.rAWBInfo.rPGN.i4R;

        MUINT16 idx = m_rIdxCache.getCacheValue(EModule_LCE, eStage, IDXCACHE_VALTYPE_CURRENT);
        MUINT16 GGMidx = m_rIdxCache.getCacheValue(EModule_GMA, eStage, IDXCACHE_VALTYPE_CURRENT);

        AAA_TRACE_ALG(calculateLCE);
        m_pLceMgr->calculateLCE(rIspCamInfo, idx, GGMidx, (MUINT16*)m_pLCESBuffer->getImageBufferHeap()->getBufVA(0),
                                &lce, &m_LceExifInfo,
                                m_bSlave, m_ISP_Sync_Info);
        AAA_TRACE_END_ALG;

        if(isDynamicTuning()){ /*m_pIspTuningCustom->refine_MIX(m_rIspCamInfo, m_IspNvramMgr, mix); */ }

        rLce.put(SubModuleIndex, lce);
    }
    else{
        rLce.setEnable(SubModuleIndex, MFALSE);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DCES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_DCES(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{

    //DCES Part
    ISP_MGR_DCES_T& rDces = ISP_MGR_DCES_T::getInstance(m_eSensorDev);

    if(rDces.isEnable(SubModuleIndex)){
       rDces.setLastYV_Bld(SubModuleIndex, m_pDceMgr->getLastDCES_yv_bld());
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getNvram_PerFrame_DCE(MUINT8 SubModuleIndex, const RAWIspCamInfo& rIspCamInfo, IDXCACHE_STAGE eStage)
{
    ISP_MGR_DCE_T& rDce = ISP_MGR_DCE_T::getInstance(m_eSensorDev);
    ISP_MGR_FLC_T& rFlc = ISP_MGR_FLC_T::getInstance(m_eSensorDev);

    if(rDce.isEnable(SubModuleIndex)){

        ISP_NVRAM_DCE_T dce ={0};
        MINT32 i4DRINDEX_L = 0;
        MINT32 i4DRINDEX_H = 0;
        m_pLceMgr->getLastDCE_IDX_Info(i4DRINDEX_L, i4DRINDEX_H);

        MUINT16 idxDCE = m_rIdxCache.getCacheValue(EModule_DCE, eStage, IDXCACHE_VALTYPE_CURRENT);

        AAA_TRACE_ALG(calculateDCE);
        if(m_pDCESBuffer){
            m_pDceMgr->calculateDCE(rIspCamInfo, idxDCE, (MUINT32*)m_pDCESBuffer->getImageBufferHeap()->getBufVA(0),
                                    i4DRINDEX_L, i4DRINDEX_H, &dce, &m_DceExifInfo);
        }
        else{
            m_pDceMgr->calculateDCE(rIspCamInfo, idxDCE, NULL,
                                    i4DRINDEX_L, i4DRINDEX_H, &dce, &m_DceExifInfo);
        }
        AAA_TRACE_END_ALG;

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            //m_pIspTuningCustom->refine_LCE(rIspCamInfo, m_IspNvramMgr, lce);
        }
        // Load it to ISP manager buffer.

        rDce.put(ISP_MGR_DCE::EDCE_D1, dce);


        MINT32 captureDump = property_get_int32("vendor.isp.dce_tbl.dump", 0);
        MINT32 previewDump = property_get_int32("vendor.isp.dce_tbl.dump.preview", 0);
        MBOOL bDumpTbl = MFALSE;
        if ((!rIspCamInfo.fgRPGEnable && captureDump) || previewDump)
            bDumpTbl = MTRUE;

        if (bDumpTbl)
        {
            MBOOL bDCESO_Valid = m_pDCESBuffer
                                 && (m_pDCESBuffer->getImageBufferHeap())
                                 && (m_pDCESBuffer->getImageBufferHeap()->getBufVA(0));

            char filename[512] = "";
            genFileName_DCESO(filename, sizeof(filename), &m_hint_p2);

            if(bDCESO_Valid){
                m_pDCESBuffer->saveToFile(filename);
            }
            else{
                FILE* pFileTemp = fopen(filename, "w");
                if(pFileTemp) fclose(pFileTemp);
            }
         }
    }

    //Dump LTMSO
    {
        MINT32 captureDump = property_get_int32("vendor.isp.ltm_tbl.dump", 0);
        MINT32 previewDump = property_get_int32("vendor.isp.ltm_tbl.dump.preview", 0);
        MBOOL bDumpTbl = MFALSE;
        if ((!rIspCamInfo.fgRPGEnable && captureDump && (rIspCamInfo.bEnableNDD || (rIspCamInfo.i4P2InImgFmt != EYuv2Yuv)))
            || previewDump)
            bDumpTbl = MTRUE;

        if (bDumpTbl)
        {
                char filename[512] = "";

                genFileName_LTMSO(filename, sizeof(filename), &m_hint_p2);
                android::sp<IFileCache> fidLtmsoDump;
                fidLtmsoDump = IFileCache::open(filename);
                if (fidLtmsoDump->write(&(rIspCamInfo.rCCU_Result.LTM.LTMSO[0]), sizeof(rIspCamInfo.rCCU_Result.LTM.LTMSO)) != sizeof(rIspCamInfo.rCCU_Result.LTM.LTMSO))
                    CAM_LOGW("[%s] fail dump to %s", __FUNCTION__, filename);

                genFileName_LTM_CURVE(filename, sizeof(filename), &m_hint_p2);
                ISP_NVRAM_LTM_CURVE_T ltm_curve = rIspCamInfo.rCCU_Result.LTM.LTM_Curve;
                android::sp<IFileCache> fidLtmCurve;
                fidLtmCurve = IFileCache::open(filename);
                if (fidLtmCurve->write(&ltm_curve, sizeof(ISP_NVRAM_LTM_CURVE_T)) != sizeof(ISP_NVRAM_LTM_CURVE_T))
                    CAM_LOGW("[%s] fail dump to %s", __FUNCTION__, filename);

        }
    }
    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC_2(MUINT8 SubModuleIndex, const ISP_INFO_T& rIspInfo)
{
#if 1
    ISP_MGR_LSC_T& rLsc2 = ISP_MGR_LSC_T::getInstance(m_eSensorDev);

    MBOOL rLscP2En = rLsc2.isEnable(1);
    MBOOL fgRet = MFALSE;

    if (rLscP2En && rIspInfo.rLscData.size())
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
                m_pLscMgr->setOnOffP2(MTRUE);

                CAM_LOGD_IF(m_bDebugEnable, "[%s] Process Lsc Data @(%d), Modify Size(%dx%d -> %dx%d), grid(%dx%d), HwRto(%d)", __FUNCTION__,
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

                //get buffer from LscMgr
                ILscBuf* pLscBuf = m_pLscMgr->getP2Buf();
                rLsc2.putBuf(1, *pLscBuf);

                MINT32 captureDump = property_get_int32("vendor.debug.camera.dump.lsc2", 0);
                MINT32 previewDump = property_get_int32("vendor.debug.camera.dump.lsc2.preview", 0);

                FileReadRule rule;
                MBOOL bdumplsc = rule.isDumpEnable("3AHAL")&&
                ((rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_Capture) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_After_Blend) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Single) ||
                (rIspInfo.rCamInfo.rMapping_Info.eIspProfile == NSIspTuning::EIspProfile_MFNR_Before_Blend));

                if (rIspInfo.isCapture && captureDump && (rIspInfo.rCamInfo.bEnableNDD || (rIspInfo.rCamInfo.i4P2InImgFmt != EYuv2Yuv))
                    || previewDump)
                {
                    char strlscdump[512] = {'\0'};
                    genFileName_LSC2(strlscdump, sizeof(strlscdump), &rIspInfo.hint);

                    android::sp<IFileCache> fidLscDump;
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
    }

    if (fgRet == MFALSE)
    {
        rLsc2.enableLsc(SubModuleIndex, MFALSE);
        m_pLscMgr->setOnOffP2(MFALSE);
    }
    else
    {
        rLsc2.enableLsc(SubModuleIndex, MTRUE);
        m_pLscMgr->setOnOffP2(MTRUE);
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] lsc2(%d), table size (%d),fgRet(%d)", __FUNCTION__, rLscP2En, rIspInfo.rLscData.size(), fgRet);

#endif
    return MTRUE;
}

