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
#include <aaa_log.h>
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
#include <lib3a/dynamic_ccm.h>
//#include <lib3a/isp_interpolation.h>
#include <isp_interpolation.h>
#include <ccm_mgr.h>
//#include <lsc_mgr2.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include "paramctrl_dftP2.h"
//#include <mtkcam/hwutils/CameraProfile.h>
//#include "mtkcam/featureio/vfb_hal_base.h"
//#include <mtkcam/featureio/capturenr.h>
#include <mtkcam/utils/sys/IFileCache.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>

//define log control
#define EN_3A_FLOW_LOG        1
#define EN_3A_SCHEDULE_LOG    2

using namespace android;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_All(MINT32 const i4FrameID)
{
    MBOOL fgRet = MTRUE;

    MY_LOG_IF(m_bDebugEnable, "[%s] + i4FrameID(%d)", __FUNCTION__, i4FrameID);

    TuningMgr::getInstance().dequeBuffer(m_eSoftwareScenario, i4FrameID);

    fgRet = ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_SL2_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_CFA_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        ;

    TuningMgr::getInstance().enqueBuffer(m_eSoftwareScenario, i4FrameID);

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
applyToHw_PerFrame_P2(MINT32 flowType, const RAWIspCamInfo& rIspCamInfo, void* pRegBuf)
{
    MBOOL fgRet = MTRUE;

    MY_LOG_IF(m_bDebugEnable, "[%s] + pRegBuf(%p)", __FUNCTION__, pRegBuf);

    isp_reg_t* pReg = reinterpret_cast<isp_reg_t*>(pRegBuf);

    if (flowType == 1)
    {
        MY_LOG("[%s] PureRaw Flow", __FUNCTION__);
        fgRet = ISP_MGR_OBC2_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
                &&  ISP_MGR_DBS2_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
                &&  ISP_MGR_BNR2_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
                &&  ISP_MGR_LSC2_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
                //&&  ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
                ;
    }

    fgRet = ISP_MGR_SL2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg, rIspCamInfo.rRrzInfo)
        &&  ISP_MGR_NSL2A_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg, rIspCamInfo.rRrzInfo)
        &&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_CFA_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PCA_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        //&&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        //&&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        //&&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        ;

    MBOOL enDftP2 = propertyGet("vendor.isp.defaultP2", 0);
    if(enDftP2==1)
        memcpy((MUINT8 *)(&pReg->CAM_CTL_START), (MUINT8 *)p2_tuning_buffer, sizeof(isp_reg_t)-0x4000);

    MY_LOG_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);
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
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
        //&&  ISP_MGR_CFA_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).reset()
        //&&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).reset()
        ;

    if  ( ! fgRet )
    {
        MY_ERR("reset error");
        goto lbExit;
    }

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_DBS()
        &&  prepareHw_PerFrame_OBC()
        &&  prepareHw_PerFrame_BPC()
        &&  prepareHw_PerFrame_NR1()
        &&  prepareHw_PerFrame_PDC()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_RPG()
        //&&  prepareHw_PerFrame_PGN()
        //&&  prepareHw_PerFrame_CFA()
        &&  prepareHw_PerFrame_CCM()
        //&&  prepareHw_PerFrame_GGM()
        //&&  prepareHw_PerFrame_ANR()
        //&&  prepareHw_PerFrame_CCR()
        //&&  prepareHw_PerFrame_PCA()
        //&&  prepareHw_PerFrame_EE()
        //&&  prepareHw_PerFrame_EFFECT()
        //&&  prepareHw_PerFrame_NR3D()
        //&&  prepareHw_PerFrame_MFB()
        //&&  prepareHw_PerFrame_MIXER3()
        ;

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_PREPARE, CPTFlagEnd);   // Profiling End.

    if  ( ! fgRet )
    {
        MY_ERR("prepareHw error");
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
    MY_LOG_IF(m_bDebugEnable, "[%s] +", __FUNCTION__);

    MBOOL fgRet = MTRUE
        &&  prepareHw_PerFrame_OBC2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_DBS2(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_BPC2(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_PGN(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_CFA(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_CCM(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_GGM(rIspInfo.rCamInfo)
        //&&  prepareHw_PerFrame_GGM(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_NSL2A(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_ANR(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_CCR(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_PCA(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_EE(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_EFFECT(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_LSC_2(rIspInfo)
        //&&  prepareHw_PerFrame_NR3D(rIspCamInfo, rIdx)
        //&&  prepareHw_PerFrame_MFB(rIspCamInfo, rIdx)
        //&&  prepareHw_PerFrame_MIXER3(rIspCamInfo, rIdx)
        ;

    MY_LOG_IF(m_bDebugEnable, "[%s] -", __FUNCTION__);

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
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
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
        &&  prepareHw_DynamicBypass_OBC()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_PGN()
        &&  prepareHw_PerFrame_CFA()
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
    MY_LOG_IF(m_bDebugEnable, "[prepareHw_PerFrame_Partial()] exit\n");
    return  fgRet;

}

MBOOL
Paramctrl::
prepareHw_PerFrame_Partial(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    RAWIspCamInfo temp = rIspCamInfo;

    prepareHw_PerFrame_CFA(temp, rIdx);
    return MTRUE;
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
            //AAA_TRACE_LSC(evaluate_Shading_Ratio);
            i4Rto = m_pIspTuningCustom->evaluate_Shading_Ratio(m_rIspCamInfo);
            //AAA_TRACE_END_LSC;
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

    MY_LOG_IF(m_bDebugEnable, "[%s] (flash, iso, rto)=(%d, %d, %d), auto(%d), lock(%d,%d), dyn(%d), op(%d), PreCapFlashIsLockRto(%d), fgForceUnlock(%d)",
        __FUNCTION__, m_rIspCamInfo.rFlashInfo.isFlash, m_rIspCamInfo.rAEInfo.u4RealISOValue, i4Rto, fgAutoRto,
        m_rIspCamInfo.rAEInfo.bAELock, m_rIspCamInfo.rAEInfo.bAELimiter, m_fgDynamicShading, getOperMode(), i4IsLockRto, fgForceUnlock);

    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    //////////////////////////////////////
    return  MTRUE;
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
prepareHw_PerFrame_DBS()
{
    MBOOL bDisable = propertyGet("vendor.isp.dbs.disable", 0);

    if ((bDisable) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (!ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTEnable())){
        ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS();

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx = m_IspNvramMgr.getIdx_DBS()+1;
                    m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = m_IspNvramMgr.getIdx_DBS();

                    m_ISP_INT.u4DbsUpperISO = m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO;
                    m_ISP_INT.u4DbsLowerISO = m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO;
                    m_ISP_INT.u2DbsUpperIdx = m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx;
                    m_ISP_INT.u2DbsLowerIdx = m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx;


                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx);

                    SmoothDBS(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO, // Lower ISO
                             m_IspNvramMgr.getDBS(m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx), // DBS settings for upper ISO
                             m_IspNvramMgr.getDBS(m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx), // DBS settings for lower ISO
                             dbs);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx = m_IspNvramMgr.getIdx_DBS();
                    if (m_IspNvramMgr.getIdx_DBS() != 0){
                        m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = m_IspNvramMgr.getIdx_DBS()-1;
                    } else {
                        m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = m_IspNvramMgr.getIdx_DBS();
                    }

                    m_ISP_INT.u4DbsUpperISO = m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO;
                    m_ISP_INT.u4DbsLowerISO = m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO;
                    m_ISP_INT.u2DbsUpperIdx = m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx;
                    m_ISP_INT.u2DbsLowerIdx = m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx);

                    SmoothDBS(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO, // Lower ISO
                             m_IspNvramMgr.getDBS(m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx), // DBS settings for upper ISO
                             m_IspNvramMgr.getDBS(m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx), // DBS settings for lower ISO
                             dbs);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_DBS(m_rIspCamInfo, m_IspNvramMgr, dbs);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, dbs);
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

    if ((bDisable) ||
        (!ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTEnable())){
        ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC(m_ParamIdx_P1.OBC);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_OBC(m_rIspCamInfo, m_IspNvramMgr, obc);
        }

        this->setPureOBCInfo(&obc);

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, obc );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_DynamicBypass_OBC()
{
    // Get backup NVRAM parameter
    ISP_NVRAM_OBC_T obc;

    this->getPureOBCInfo(&obc);

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, m_eSensorTG, obc );

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

    if ((bDisable) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTBPCEnable())){
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_BPC_T bpc = m_IspNvramMgr.getBPC(m_ParamIdx_P1.BNR_BPC);

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx = m_IspNvramMgr.getIdx_BPC()+1;
                    m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = m_IspNvramMgr.getIdx_BPC();

                    m_ISP_INT.u4BpcUpperISO = m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO;
                    m_ISP_INT.u4BpcLowerISO = m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO;
                    m_ISP_INT.u2BpcUpperIdx = m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx;
                    m_ISP_INT.u2BpcLowerIdx = m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx;


                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx);

                    SmoothBPC(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO, // Lower ISO
                             m_IspNvramMgr.getBPC(m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx), // BPC settings for upper ISO
                             m_IspNvramMgr.getBPC(m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx), // BPC settings for lower ISO
                             bpc);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx = m_IspNvramMgr.getIdx_BPC();
                    if (m_IspNvramMgr.getIdx_BPC() != 0){
                        m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = m_IspNvramMgr.getIdx_BPC()-1;
                    } else {
                        m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = m_IspNvramMgr.getIdx_BPC();
                    }
                    m_ISP_INT.u4BpcUpperISO = m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO;
                    m_ISP_INT.u4BpcLowerISO = m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO;
                    m_ISP_INT.u2BpcUpperIdx = m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx;
                    m_ISP_INT.u2BpcLowerIdx = m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx);

                    SmoothBPC(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO, // Lower ISO
                             m_IspNvramMgr.getBPC(m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx), // BPC settings for upper ISO
                             m_IspNvramMgr.getBPC(m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx), // BPC settings for lower ISO
                             bpc);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BPC(m_rIspCamInfo, m_IspNvramMgr, bpc);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, bpc );
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

    if ((bDisable) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTCTEnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)) {
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setCTEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setCTEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_NR1_T nr1 = m_IspNvramMgr.getNR1(m_ParamIdx_P1.BNR_NR1);

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx = m_IspNvramMgr.getIdx_NR1()+1;
                    m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx = m_IspNvramMgr.getIdx_NR1();

                    m_ISP_INT.u4Nr1UpperISO = m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO;
                    m_ISP_INT.u4Nr1LowerISO = m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO;
                    m_ISP_INT.u2Nr1UpperIdx = m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx;
                    m_ISP_INT.u2Nr1LowerIdx = m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx);

                    SmoothNR1(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO, // Lower ISO
                             m_IspNvramMgr.getNR1(m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx), // NR1 settings for upper ISO
                             m_IspNvramMgr.getNR1(m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx), // NR1 settings for lower ISO
                             nr1);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx = m_IspNvramMgr.getIdx_NR1();
                    if (m_IspNvramMgr.getIdx_NR1() != 0){
                        m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx = m_IspNvramMgr.getIdx_NR1()-1;
                    } else {
                        m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx = m_IspNvramMgr.getIdx_NR1();
                    }

                    m_ISP_INT.u4Nr1UpperISO = m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO;
                    m_ISP_INT.u4Nr1LowerISO = m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO;
                    m_ISP_INT.u2Nr1UpperIdx = m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx;
                    m_ISP_INT.u2Nr1LowerIdx = m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx);

                    SmoothNR1(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO, // Lower ISO
                             m_IspNvramMgr.getNR1(m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx), // NR1 settings for upper ISO
                             m_IspNvramMgr.getNR1(m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx), // NR1 settings for lower ISO
                             nr1);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_NR1(m_rIspCamInfo, m_IspNvramMgr, nr1);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, nr1 );
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

    if (bDisable  ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTCTEnable()) ||
        (m_rIspCamInfo.eSensorMode != ESensorMode_Capture)) {
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setPDCEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setPDCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_PDC_T pdc = m_IspNvramMgr.getPDC(m_ParamIdx_P1.BNR_PDC);

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx = m_IspNvramMgr.getIdx_PDC()+1;
                    m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx = m_IspNvramMgr.getIdx_PDC();

                    m_ISP_INT.u4PdcUpperISO = m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO;
                    m_ISP_INT.u4PdcLowerISO = m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO;
                    m_ISP_INT.u2PdcUpperIdx = m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx;
                    m_ISP_INT.u2PdcLowerIdx = m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx);

                    SmoothPDC(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO, // Lower ISO
                             m_IspNvramMgr.getPDC(m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx), // PDC settings for upper ISO
                             m_IspNvramMgr.getPDC(m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx), // PDC settings for lower ISO
                             pdc);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx = m_IspNvramMgr.getIdx_PDC();
                    if (m_IspNvramMgr.getIdx_PDC() != 0){
                        m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx = m_IspNvramMgr.getIdx_PDC()-1;
                    } else {
                        m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx = m_IspNvramMgr.getIdx_PDC();
                    }

                    m_ISP_INT.u4PdcUpperISO = m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO;
                    m_ISP_INT.u4PdcLowerISO = m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO;
                    m_ISP_INT.u2PdcUpperIdx = m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx;
                    m_ISP_INT.u2PdcLowerIdx = m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx);

                    SmoothPDC(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO, // Lower ISO
                             m_IspNvramMgr.getPDC(m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx), // PDC settings for upper ISO
                             m_IspNvramMgr.getPDC(m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx), // PDC settings for lower ISO
                             pdc);  // Output
                 }
             }
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_PDC(m_rIspCamInfo, m_IspNvramMgr, pdc);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, pdc );
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
    MY_LOG_IF(m_bDebugEnable,"%s", __FUNCTION__);
    MINT32 defaultLSCValue = -1;
#if CAM3_DISABLE_SHADING
    defaultLSCValue = 0;
#endif
    //////////////////////////////////////
    MUINT32 new_cct_idx = eIDX_Shading_CCT_BEGIN;

    // Check to see if it is needed to load LUT.
    MY_LOG_IF(m_bDebugEnable,"[%s] m_pLscMgr OperMode(%d)\n", __FUNCTION__, getOperMode());

    // Invoke callback for customers to modify.
    if  (m_fgDynamicShading)
    {
        // Dynamic Tuning: Enable
        new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo);
        m_pLscMgr->setCTIdx(new_cct_idx);
    }

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

    /* temporally disable
    //get SL2 from LSC
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
    */
    //////////////////////////////////////


    // debug message
    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    m_IspNvramMgr.setIdx_LSC(m_pLscMgr->getRegIdx());
    //////////////////////////////////////
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_SL2()
{
    // Get default NVRAM parameter
    ISP_NVRAM_SL2_T sl2;

    getIspHWBuf(m_eSensorDev, sl2 );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_SL2(m_rIspCamInfo, m_IspNvramMgr, sl2);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, sl2 );

    return  MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_NSL2A()
{
    // Get default NVRAM parameter
    MUINT16 u2Idx = (m_rIspCamInfo.eSensorMode*SHADING_SUPPORT_CT_NUM+ (UINT16)m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo));
    m_ParamIdx_P1.SL2 = u2Idx;
    ISP_NVRAM_NSL2A_T nsl2a = m_IspNvramMgr.getSL2(u2Idx);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NSL2A(m_rIspCamInfo, m_IspNvramMgr, nsl2a);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, nsl2a );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RPG()
{
    MBOOL bDisable = propertyGet("vendor.isp.rpg.disable", 0);

    // Get default NVRAM parameter
    ISP_NVRAM_RPG_T rpg;

    AWB_GAIN_T rCurrentAWBGain = m_rIspCamInfo.rAWBInfo.rCurrentAWBGain;

    ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAWBGain(rCurrentAWBGain);

    if (isRPGEnable() && (!bDisable)) {// RPG is enabled
        ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        getIspHWBuf(m_eSensorDev, m_eSensorTG, rpg );

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_RPG(m_rIspCamInfo, m_IspNvramMgr, rpg);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, m_eSensorTG, rpg );
    }
    else {
        ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN()
{
    MBOOL bDisable = propertyGet("vendor.isp.pgn.disable", 0);

    MY_LOG_IF(m_bDebugEnable,"%s(): isRPGEnable() = %d\n", __FUNCTION__, isRPGEnable());

    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    if (isRPGEnable() || (bDisable)) {// RPG is enabled
        ISP_MGR_PGN_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else
    {
        ISP_MGR_PGN_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        getIspHWBuf(m_eSensorDev, pgn );

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_PGN(m_rIspCamInfo, m_IspNvramMgr, pgn);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, pgn );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CFA()
{
    MBOOL bDisable = propertyGet("vendor.isp.cfa.disable", 0);

    if ((m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        return MTRUE;
    }

    // Get default NVRAM parameter
    ISP_NVRAM_CFA_T cfa = m_IspNvramMgr.getCFA();

    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
        if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
             if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx = m_IspNvramMgr.getIdx_CFA()+1;
                m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx = m_IspNvramMgr.getIdx_CFA();

                m_ISP_INT.u4CfaUpperISO = m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO;
                m_ISP_INT.u4CfaLowerISO = m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO;
                m_ISP_INT.u2CfaUpperIdx = m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx;
                m_ISP_INT.u2CfaLowerIdx = m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx;

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                          m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO,
                          m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO,
                          m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx,
                          m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);

                SmoothCFA(m_rIspCamInfo.u4ISOValue,  // Real ISO
                          m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO, // Upper ISO
                          m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO, // Lower ISO
                          m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx), // CFA settings for upper ISO
                          m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx), // CFA settings for lower ISO
                          cfa);  // Output
             }
         }
         else {
             if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx = m_IspNvramMgr.getIdx_CFA();
                if (m_IspNvramMgr.getIdx_CFA() != 0){
                    m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx = m_IspNvramMgr.getIdx_CFA()-1;
                } else {
                    m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx = m_IspNvramMgr.getIdx_CFA();
                }

                m_ISP_INT.u4CfaUpperISO = m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO;
                m_ISP_INT.u4CfaLowerISO = m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO;
                m_ISP_INT.u2CfaUpperIdx = m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx;
                m_ISP_INT.u2CfaLowerIdx = m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx;

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                          m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO,
                          m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO,
                          m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx,
                          m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);

                SmoothCFA(m_rIspCamInfo.u4ISOValue,  // Real ISO
                m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO, // Upper ISO
                m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO, // Lower ISO
                m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx), // CFA settings for upper ISO
                m_IspNvramMgr.getCFA(m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx), // CFA settings for lower ISO
                cfa);  // Output
             }
         }
    }

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CFA(m_rIspCamInfo, m_IspNvramMgr, cfa);
    }

    // FG mode protection
    if (isRPGEnable() && ((m_rIspCamInfo.eIspProfile == EIspProfile_Preview)      ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_Video)        ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_N3D_Preview)  ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_N3D_Video)    ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
                          (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video))) {
        cfa.byp.bits.DM_FG_MODE = 1;
    }
    else {
        cfa.byp.bits.DM_FG_MODE = 0;
    }

    if ((!ISP_MGR_CFA_T::getInstance(m_eSensorDev).isCCTEnable()) || (bDisable)) // CCT usage: fix CFA index
    {
        cfa.byp.bits.DM_BYP = 1;
        //cfa = m_IspNvramMgr.getCFA(NVRAM_CFA_DISABLE_IDX);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, cfa );

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

    if ((bDisable) ||
        (!ISP_MGR_CCM_T::getInstance(m_eSensorDev).isCCTEnable())              ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCM_T ccm = m_pCcmMgr->getCCM();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm);
        }

        // Load it to ISP manager buffer.
        //putIspHWBuf(m_eSensorDev, ccm );
        m_rIspCamInfo.rMtkCCM = ccm;
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
    MBOOL bDisable = propertyGet("vendor.isp.ggm.disable", 0);

    ISP_NVRAM_GGM_T ggm;

    if ((bDisable) ||
        (!ISP_MGR_GGM_T::getInstance(m_eSensorDev).isCCTEnable())              ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MTRUE);
        if (rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
        {
            ggm = m_pGgmMgr->getGGM();
        }
        else
        {
            // Get default NVRAM parameter
            if ((rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
                (rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
                (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
                (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video)) {
                ggm = m_IspNvramMgr.getIHDRGGM(rIspCamInfo.rAEInfo.i4GammaIdx);
                MY_LOG_IF(m_bDebugEnable,"%s: rIspCamInfo.rAEInfo.i4GammaIdx = %d", __FUNCTION__, rIspCamInfo.rAEInfo.i4GammaIdx);
            }
            else {
                ggm = m_IspNvramMgr.getGGM();
            }

            // Invoke callback for customers to modify.
            if ( isDynamicTuning() )
            {   //  Dynamic Tuning: Enable
                m_pIspTuningCustom->refine_GGM(rIspCamInfo,  m_IspNvramMgr, ggm);

                m_pIspTuningCustom->userSetting_EFFECT_GGM(rIspCamInfo, rIspCamInfo.eIdx_Effect, ggm);
            }

            // Load it to ISP manager buffer.
            //putIspHWBuf(m_eSensorDev, ggm );
            m_pGgmMgr->updateGGM(ggm);
        }
        putIspHWBuf(m_eSensorDev, ggm);
    }
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR()
{
    MBOOL bDisable = propertyGet("vendor.isp.anr.disable", 0);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(m_rIspCamInfo);

    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)) {

        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANREnable(MFALSE);
    }
    else if ((m_rIspCamInfo.eIspProfile == EIspProfile_Capture_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_PureRAW_Capture_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR) ||
             (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {

        if (m_rIspCamInfo.u4ISOValue >= m_u4SwnrEncEnableIsoThreshold) {
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENCEnable(MTRUE);
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENYEnable(MTRUE);
        }
        else {
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENCEnable(MFALSE);
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENYEnable(MTRUE);
        }

        // Get default NVRAM parameter
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR()+1;
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();

                    m_ISP_INT.u4AnrUpperISO = m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO;
                    m_ISP_INT.u4AnrLowerISO = m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO;
                    m_ISP_INT.u2AnrUpperIdx = m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx;
                    m_ISP_INT.u2AnrLowerIdx = m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_rIspCamInfo.u4ISOValue,  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR();
                    if (m_IspNvramMgr.getIdx_ANR() != 0){
                        m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;
                    } else {
                        m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();
                    }

                    m_ISP_INT.u4AnrUpperISO = m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO;
                    m_ISP_INT.u4AnrLowerISO = m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO;
                    m_ISP_INT.u2AnrUpperIdx = m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx;
                    m_ISP_INT.u2AnrLowerIdx = m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_rIspCamInfo.u4ISOValue,  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(m_rIspCamInfo, m_IspNvramMgr, anr);
        }

        // set ANR_LCE_LINK
//#warning "SWNR linker error"
#if 0
        SwNRParam::getInstance(m_i4SensorIdx)->setANR_LCE_LINK(static_cast<MBOOL>(anr.con1.bits.ANR_LCE_LINK));
        m_pLscMgr->setSwNr();
#endif
        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
    }
    else {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR()+1;
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();

                    m_ISP_INT.u4AnrUpperISO = m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO;
                    m_ISP_INT.u4AnrLowerISO = m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO;
                    m_ISP_INT.u2AnrUpperIdx = m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx;
                    m_ISP_INT.u2AnrLowerIdx = m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_rIspCamInfo.u4ISOValue,  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR();
                    if (m_IspNvramMgr.getIdx_ANR() != 0) {
                        m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;
                    } else {
                        m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();
                    }

                    m_ISP_INT.u4AnrUpperISO = m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO;
                    m_ISP_INT.u4AnrLowerISO = m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO;
                    m_ISP_INT.u2AnrUpperIdx = m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx;
                    m_ISP_INT.u2AnrLowerIdx = m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_rIspCamInfo.u4ISOValue,  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(m_rIspCamInfo, m_IspNvramMgr, anr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
    }

    //ANR_TBL
     bDisable = propertyGet("vendor.isp.anr_tbl.disable", 0);
     ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANRTBLEnable(!bDisable);

     m_rIspCamInfo.eIdx_ANR_TBL_ISO = m_pIspTuningCustom->evaluate_ANR_TBL_ISO_index(m_rIspCamInfo);

     ISP_MGR_NBC_T::getInstance(m_eSensorDev).putANR_TBL(m_rIspParam.ANR_TBL[m_rIspCamInfo.eIdx_ANR_TBL_CT][m_rIspCamInfo.eIdx_ANR_TBL_ISO]);

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR()
{
    MBOOL bDisable = propertyGet("vendor.isp.ccr.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTCCREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)       ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setCCREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCR(m_rIspCamInfo, m_IspNvramMgr, ccr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ccr );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PCA()
{
    MBOOL bDisable = propertyGet("vendor.isp.pca.disable", 0);

    //MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

    if (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
        m_ePCAMode = EPCAMode_360BIN;
    }
    else {
        m_ePCAMode = EPCAMode_180BIN;
    }

    if ((bDisable) ||
        (!ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).isCCTEnable())  ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MTRUE);
        //dynamic color
        ISP_NVRAM_PCA_LUT_T pca = m_rIspParam.ISPPca.PCA_LUT[0][m_rIspCamInfo.eIdx_CT];

        MBOOL bIsCapture = m_pIspTuningCustom->is_to_invoke_capture_in_color(m_rIspCamInfo);

        if (m_rIspParam.ISPPca.COLOR_Method &&
            (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)))
        {// Smooth PCA
            SmoothPCA_TBL(  bIsCapture,
                            m_rIspParam.ISPPca.COLOR_CT_Env[m_rIspCamInfo.eIdx_CT_U],
                            m_rIspParam.ISPPca.COLOR_CT_Env[m_rIspCamInfo.eIdx_CT_L],
                            m_rIspCamInfo.eIdx_CT_U, m_rIspCamInfo.eIdx_CT_L,
                            m_rIspParam.ISPPca.PCA_LUT,
                            m_rIspParam.ISPPca.rPcaParam,
                            m_rIspCamInfo,
                            pca,
                            0,
                            m_bSmoothPCA_FirstTimeBoot,
                            m_SmoothPCA_Prv);
        }

        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(reinterpret_cast<MUINT32*>(&pca.lut[0]));
        m_pPcaMgr->loadConfig();
    }

#if 0
    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

    if (m_ePCAMode == EPCAMode_360BIN) { // for VFB
#warning "VFB linker error"
#if 0
        MY_LOG_IF(m_bDebugEnable,"%s: loading vFB PCA (0x%x) ...", __FUNCTION__, halVFBTuning::getInstance().mHalVFBTuningGetPCA());
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(halVFBTuning::getInstance().mHalVFBTuningGetPCA());
#endif
    }
    else {
        // (2) Invoke callback for customers to modify.
        if  (isDynamicTuning())
        {   // Dynamic Tuning: Enable
            //m_pPcaMgr->setIdx(static_cast<MUINT32>(m_rIspCamInfo.eIdx_PCA_LUT));
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
            ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(reinterpret_cast<MUINT32*>(&pca.lut[0]));
            m_pPcaMgr->loadConfig();
        }
    }
#endif
    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EE()
{
    MBOOL bDisable = propertyGet("vendor.isp.ee.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_SEEE_T::getInstance(m_eSensorDev).isCCTEnable())             ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off)          ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE();

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo)) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4EeUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4EeLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx = m_IspNvramMgr.getIdx_EE()+1;
                    m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx = m_IspNvramMgr.getIdx_EE();

                    m_ISP_INT.u4EeUpperISO = m_rIspCamInfo.rIspIntInfo.u4EeUpperISO;
                    m_ISP_INT.u4EeLowerISO = m_rIspCamInfo.rIspIntInfo.u4EeLowerISO;
                    m_ISP_INT.u2EeUpperIdx = m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx;
                    m_ISP_INT.u2EeLowerIdx = m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4EeUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4EeLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx);

                    SmoothEE(m_rIspCamInfo.u4ISOValue,  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4EeUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4EeLowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4EeUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4EeLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx = m_IspNvramMgr.getIdx_EE();
                    if (m_IspNvramMgr.getIdx_EE() != 0){
                        m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx = m_IspNvramMgr.getIdx_EE()-1;
                    } else {
                        m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx = m_IspNvramMgr.getIdx_EE();
                    }

                    m_ISP_INT.u4EeUpperISO = m_rIspCamInfo.rIspIntInfo.u4EeUpperISO;
                    m_ISP_INT.u4EeLowerISO = m_rIspCamInfo.rIspIntInfo.u4EeLowerISO;
                    m_ISP_INT.u2EeUpperIdx = m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx;
                    m_ISP_INT.u2EeLowerIdx = m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4EeUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4EeLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx);

                    SmoothEE(m_rIspCamInfo.u4ISOValue,  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4EeUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4EeLowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_EE(m_rIspCamInfo, m_IspNvramMgr, ee);

            if (m_rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
            {
                // User setting
                m_pIspTuningCustom->userSetting_EE(m_rIspCamInfo, m_rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);
            }
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ee);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EFFECT()
{
    MBOOL bDisable = propertyGet("vendor.isp.g2c.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_G2C_T::getInstance(m_eSensorDev).isCCTEnable())              ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta) {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MTRUE);
        //}

        ISP_NVRAM_G2C_T g2c;
        ISP_NVRAM_G2C_SHADE_T g2c_shade;
        ISP_NVRAM_SE_T se;
        ISP_NVRAM_GGM_T ggm;

        // Get ISP HW buffer
        getIspHWBuf(m_eSensorDev, g2c);
        getIspHWBuf(m_eSensorDev, g2c_shade);
        getIspHWBuf(m_eSensorDev, se);
//return to GGM setting        getIspHWBuf(m_eSensorDev, ggm);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfo, m_rIspCamInfo.eIdx_Effect, m_rIspCamInfo.rIspUsrSelectLevel, g2c, g2c_shade, se, ggm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, g2c);
        putIspHWBuf(m_eSensorDev, g2c_shade);
        putIspHWBuf(m_eSensorDev, se);
//return to GGM setting        putIspHWBuf(m_eSensorDev, ggm);
    }

    return  MTRUE;
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
    EIspProfile_T profile = rIspInfo.rCamInfo.eIspProfile;
    MY_LOG_IF(m_bDebugEnable, "[%s] Profile = %d, LSC data size %d", __FUNCTION__, profile, rIspInfo.rLscData.size());
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
            MY_ERR("Abnormal config (%d,%d,%d,%d,%d)", rCfg.i4GridX, rCfg.i4GridY, rCfg.i4ImgWd, rCfg.i4ImgHt, rRsvd.u4HwRto);
        }
        else
        {
            MUINT32 u4W, u4H;
            if (m_pLscMgr->getRawSize((ESensorMode_T) rIspInfo.rCamInfo.eSensorMode, u4W, u4H))
            {
                MY_LOG_IF(m_bDebugEnable, "[%s] Process Lsc Data @(%d), Modify Size(%dx%d -> %dx%d), grid(%dx%d), HwRto(%d)", __FUNCTION__,
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

                NSCam::TuningUtils::FileReadRule rule;
                MBOOL bdumplsc = rule.isDumpEnable("3AHAL")&&
                ((rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_Capture) ||
                (rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_MFNR_After_Blend) ||
                (rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_MFNR_MFB) ||
                (rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_MFNR_Single) ||
                (rIspInfo.rCamInfo.eIspProfile == NSIspTuning::EIspProfile_MFNR_Before_Blend));

                if (rIspInfo.isCapture && captureDump || previewDump)
                {
                    char strlscdump[512] = {'\0'};
                    genFileName_LSC2(strlscdump, sizeof(strlscdump), &rIspInfo.hint);

                    sp<IFileCache> fidLscDump;
                    fidLscDump = IFileCache::open(strlscdump);
                    if (fidLscDump->write(&rTbl.getConfig(), sizeof(ILscTable::Config)) != sizeof(ILscTable::Config))
                        MY_ERR("write config error to %s", strlscdump);
                    if (fidLscDump->write(&rTbl.getRsvdData().u4HwRto, sizeof(MUINT32)) != sizeof(MUINT32))
                        MY_ERR("write rsvdata error to %s", strlscdump);
                    if (fidLscDump->write(rIspInfo.rLscData.data()+sizeof(ILscTable::Config)+sizeof(MUINT32), rTbl.getSize()) != rTbl.getSize())
                        MY_ERR("write shading table error to %s", strlscdump);
                }

                fgRet = MTRUE;
            }
            else
            {
                MY_ERR("Fail to get RAW size!");
            }
        }
        //fgLsc2En = (profile != EIspProfile_N3D_Denoise) && (profile != EIspProfile_N3D_Denoise_toGGM);
        //:2185CAM_LOGD_IF(m_bDebugEnable, "[%s] fgLsc2En = %d", __FUNCTION__, fgLsc2En);
    }

    MY_LOG_IF(m_bDebugEnable, "[%s] force off(%d), lsc2(%d), pgn(%d), pureraw(%d), fgRet(%d)", __FUNCTION__, bDisable, fgLsc2En, fgPgnEn, fgPureRaw, fgRet);

    if (!fgLsc2En || bDisable || (fgRet == MFALSE)||
        (rIspInfo.rCamInfo.eIspProfile == EIspProfile_YUV_Reprocess))
    {
        rLsc2.enable(MFALSE);
    }
    else
    {
        rLsc2.enable(MTRUE);
    }

    return MTRUE;
}


#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D()
{
#if 0
    // Get default NVRAM parameter
    ISP_NVRAM_NR3D_T nr3d = m_IspNvramMgr.getNR3D();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR3D(m_rIspCamInfo, m_IspNvramMgr, nr3d);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, nr3d );
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MFB()
{
#if 0
    // Get default NVRAM parameter
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MFB(m_rIspCamInfo, m_IspNvramMgr, mfb);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, mfb );
#endif

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Mixer3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MIXER3()
{
#if 0
    // Get default NVRAM parameter
    ISP_NVRAM_MIXER3_T mixer3 = m_rIspParam.ISPMfbMixer;
//#else
    // Since the original design can't apply mixer3 by ISO, and the nvram of mfb mix has never been used.
    // We steal the nvram of mfb mix and apply it to mixer3.
    // Get default NVRAM parameter
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB();
    ISP_NVRAM_MIXER3_T mixer3;
    //
    mixer3.ctrl_0.bits.MIX3_WT_SEL = 1;
    mixer3.ctrl_0.bits.MIX3_B0 = mfb.ll_con6.bits.BLD_LL_MX_B0;
    mixer3.ctrl_0.bits.MIX3_B1 = mfb.ll_con6.bits.BLD_LL_MX_B1;
    mixer3.ctrl_0.bits.MIX3_DT = mfb.ll_con5.bits.BLD_LL_MX_DT;
    //
    mixer3.ctrl_1.bits.MIX3_M0 = mfb.ll_con5.bits.BLD_LL_MX_M0;
    mixer3.ctrl_1.bits.MIX3_M1 = mfb.ll_con5.bits.BLD_LL_MX_M1;
//#endif

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MIXER3(m_rIspCamInfo, m_IspNvramMgr, mixer3);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, mixer3 );
#endif

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LCE()
{
#if 0   //this chip doesn't have LCE
    // Get default NVRAM parameter
    ISP_NVRAM_LCE_T lce = m_IspNvramMgr.getLCE();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_LCE(m_rIspCamInfo, m_IspNvramMgr, lce);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, lce);

#endif

    return  MTRUE;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_OBC2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.obc2.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_OBC2_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTEnable())){
        ISP_MGR_OBC2_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_OBC2_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_OBC_T obc2 = m_IspNvramMgr.getOBC(m_ParamIdx_P2.OBC);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_OBC(rIspCamInfo, m_IspNvramMgr, obc2);
        }

        this->setPureOBCInfo(&obc2);

        // Load it to ISP manager buffer.
        MUINT targetGain = rIspCamInfo.rAEInfo.u4IspGain >> 1;
        MY_LOG("[%s] ISP ob index %d, isp gain %d", __FUNCTION__, m_ParamIdx_P2.OBC, targetGain);
        ISP_MGR_OBC2_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAEGain(targetGain);
        ISP_MGR_OBC2_T::getInstance(m_eSensorDev, m_eSensorTG).put(obc2);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_DBS2(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.dbs.disable", 0);

    if ((bDisable) ||
         (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
         (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (!ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTEnable())){
        ISP_MGR_DBS2_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_DBS2_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS();

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo)) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4DbsUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4DbsLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2DbsUpperIdx = m_IspNvramMgr.getIdx_DBS()+1;
                    rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = m_IspNvramMgr.getIdx_DBS();

                    m_ISP_INT.u4DbsUpperISO = rIspCamInfo.rIspIntInfo.u4DbsUpperISO;
                    m_ISP_INT.u4DbsLowerISO = rIspCamInfo.rIspIntInfo.u4DbsLowerISO;
                    m_ISP_INT.u2DbsUpperIdx = rIspCamInfo.rIspIntInfo.u2DbsUpperIdx;
                    m_ISP_INT.u2DbsLowerIdx = rIspCamInfo.rIspIntInfo.u2DbsLowerIdx;


                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4DbsUpperISO,
                              rIspCamInfo.rIspIntInfo.u4DbsLowerISO,
                              rIspCamInfo.rIspIntInfo.u2DbsUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2DbsLowerIdx);

                    SmoothDBS(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4DbsUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4DbsLowerISO, // Lower ISO
                             m_IspNvramMgr.getDBS(rIspCamInfo.rIspIntInfo.u2DbsUpperIdx), // DBS settings for upper ISO
                             m_IspNvramMgr.getDBS(rIspCamInfo.rIspIntInfo.u2DbsLowerIdx), // DBS settings for lower ISO
                             dbs);  // Output
                 }
             }
             else {
                 if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    rIspCamInfo.rIspIntInfo.u4DbsUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u4DbsLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                    rIspCamInfo.rIspIntInfo.u2DbsUpperIdx = m_IspNvramMgr.getIdx_DBS();
                    if (m_IspNvramMgr.getIdx_DBS() != 0){
                        rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = m_IspNvramMgr.getIdx_DBS()-1;
                    } else {
                        rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = m_IspNvramMgr.getIdx_DBS();
                    }

                    m_ISP_INT.u4DbsUpperISO = rIspCamInfo.rIspIntInfo.u4DbsUpperISO;
                    m_ISP_INT.u4DbsLowerISO = rIspCamInfo.rIspIntInfo.u4DbsLowerISO;
                    m_ISP_INT.u2DbsUpperIdx = rIspCamInfo.rIspIntInfo.u2DbsUpperIdx;
                    m_ISP_INT.u2DbsLowerIdx = rIspCamInfo.rIspIntInfo.u2DbsLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4DbsUpperISO,
                              rIspCamInfo.rIspIntInfo.u4DbsLowerISO,
                              rIspCamInfo.rIspIntInfo.u2DbsUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2DbsLowerIdx);

                    SmoothDBS(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4DbsUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4DbsLowerISO, // Lower ISO
                             m_IspNvramMgr.getDBS(rIspCamInfo.rIspIntInfo.u2DbsUpperIdx), // DBS settings for upper ISO
                             m_IspNvramMgr.getDBS(rIspCamInfo.rIspIntInfo.u2DbsLowerIdx), // DBS settings for lower ISO
                             dbs);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_DBS(rIspCamInfo, m_IspNvramMgr, dbs);
        }

        // Load it to ISP manager buffer.
        ISP_MGR_DBS2_T::getInstance(m_eSensorDev, m_eSensorTG).put(dbs);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_BPC2(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.bpc.disable", 0);

    if ((bDisable) ||
         (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
         (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (!ISP_MGR_BNR2_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTBPCEnable())){
        ISP_MGR_BNR2_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_BNR2_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_BPC_T bpc = m_IspNvramMgr.getBPC(m_ParamIdx_P1.BNR_BPC);

        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo)) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4BpcUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4BpcLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2BpcUpperIdx = m_IspNvramMgr.getIdx_BPC()+1;
                    rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = m_IspNvramMgr.getIdx_BPC();

                    m_ISP_INT.u4BpcUpperISO = rIspCamInfo.rIspIntInfo.u4BpcUpperISO;
                    m_ISP_INT.u4BpcLowerISO = rIspCamInfo.rIspIntInfo.u4BpcLowerISO;
                    m_ISP_INT.u2BpcUpperIdx = rIspCamInfo.rIspIntInfo.u2BpcUpperIdx;
                    m_ISP_INT.u2BpcLowerIdx = rIspCamInfo.rIspIntInfo.u2BpcLowerIdx;


                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4BpcUpperISO,
                              rIspCamInfo.rIspIntInfo.u4BpcLowerISO,
                              rIspCamInfo.rIspIntInfo.u2BpcUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2BpcLowerIdx);

                    SmoothBPC(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4BpcUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4BpcLowerISO, // Lower ISO
                             m_IspNvramMgr.getBPC(rIspCamInfo.rIspIntInfo.u2BpcUpperIdx), // BPC settings for upper ISO
                             m_IspNvramMgr.getBPC(rIspCamInfo.rIspIntInfo.u2BpcLowerIdx), // BPC settings for lower ISO
                             bpc);  // Output
                 }
             }
             else {
                 if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    rIspCamInfo.rIspIntInfo.u4BpcUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u4BpcLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                    rIspCamInfo.rIspIntInfo.u2BpcUpperIdx = m_IspNvramMgr.getIdx_BPC();
                    if (m_IspNvramMgr.getIdx_BPC() != 0){
                        rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = m_IspNvramMgr.getIdx_BPC()-1;
                    } else {
                        rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = m_IspNvramMgr.getIdx_BPC();
                    }
                    m_ISP_INT.u4BpcUpperISO = rIspCamInfo.rIspIntInfo.u4BpcUpperISO;
                    m_ISP_INT.u4BpcLowerISO = rIspCamInfo.rIspIntInfo.u4BpcLowerISO;
                    m_ISP_INT.u2BpcUpperIdx = rIspCamInfo.rIspIntInfo.u2BpcUpperIdx;
                    m_ISP_INT.u2BpcLowerIdx = rIspCamInfo.rIspIntInfo.u2BpcLowerIdx;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4BpcUpperISO,
                              rIspCamInfo.rIspIntInfo.u4BpcLowerISO,
                              rIspCamInfo.rIspIntInfo.u2BpcUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2BpcLowerIdx);

                    SmoothBPC(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4BpcUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4BpcLowerISO, // Lower ISO
                             m_IspNvramMgr.getBPC(rIspCamInfo.rIspIntInfo.u2BpcUpperIdx), // BPC settings for upper ISO
                             m_IspNvramMgr.getBPC(rIspCamInfo.rIspIntInfo.u2BpcLowerIdx), // BPC settings for lower ISO
                             bpc);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BPC(rIspCamInfo, m_IspNvramMgr, bpc);
        }

        // Load it to ISP manager buffer.
        ISP_MGR_BNR2_T::getInstance(m_eSensorDev, m_eSensorTG).put(bpc);
        //putIspHWBuf(m_eSensorDev, m_eSensorTG, bpc );
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
prepareHw_PerFrame_NSL2A(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] rIdx %p", __FUNCTION__, &rIdx);
    // Get default NVRAM parameter
    MUINT16 u2Idx = (rIspCamInfo.eSensorMode*SHADING_SUPPORT_CT_NUM+ (UINT16)m_pIspTuningCustom->evaluate_Shading_CCT_index(rIspCamInfo));
    ISP_NVRAM_NSL2A_T nsl2a = m_IspNvramMgr.getSL2(u2Idx);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NSL2A(rIspCamInfo, m_IspNvramMgr, nsl2a);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, nsl2a );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] rIdx %p", __FUNCTION__, &rIdx);
    MBOOL bDisable = propertyGet("vendor.isp.pgn.disable", 0);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;

    MY_LOG_IF(m_bDebugEnable,"%s(): fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    AWB_GAIN_T rCurrentAWBGain = rIspCamInfo.rAWBInfo.rCurrentAWBGain;

    if(rIspCamInfo.i4P2InImgFmt == 5)
    {
        rCurrentAWBGain.i4R=512;
        rCurrentAWBGain.i4G=512;
        rCurrentAWBGain.i4B=512;
    }

    ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspAWBGain(rCurrentAWBGain);


    if (fgRPGEnable || (bDisable)) {// RPG is enabled
        ISP_MGR_PGN_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else
    {
        ISP_MGR_PGN_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        getIspHWBuf(m_eSensorDev, pgn );

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_PGN(rIspCamInfo, m_IspNvramMgr, pgn);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, pgn );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CFA(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.cfa.disable", 0);
    MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;

    if ((rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        return MTRUE;
    }

    // Get default NVRAM parameter
    MUINT16 u2Idx = rIdx.getIdx_CFA();
    ISP_NVRAM_CFA_T cfa = m_IspNvramMgr.getCFA(u2Idx);
#if 0
    if (m_IspInterpCtrl
        &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo)) {
        if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
             if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                rIspCamInfo.rIspIntInfo.u4CfaUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                rIspCamInfo.rIspIntInfo.u4CfaLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                rIspCamInfo.rIspIntInfo.u2CfaUpperIdx = m_IspNvramMgr.getIdx_CFA()+1;
                rIspCamInfo.rIspIntInfo.u2CfaLowerIdx = m_IspNvramMgr.getIdx_CFA();

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, rIspCamInfo.u4ISOValue,
                          rIspCamInfo.rIspIntInfo.u4CfaUpperISO,
                          rIspCamInfo.rIspIntInfo.u4CfaLowerISO,
                          rIspCamInfo.rIspIntInfo.u2CfaUpperIdx,
                          rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);

                SmoothCFA(rIspCamInfo.u4ISOValue,  // Real ISO
                          rIspCamInfo.rIspIntInfo.u4CfaUpperISO, // Upper ISO
                          rIspCamInfo.rIspIntInfo.u4CfaLowerISO, // Lower ISO
                          m_IspNvramMgr.getCFA(rIspCamInfo.rIspIntInfo.u2CfaUpperIdx), // CFA settings for upper ISO
                          m_IspNvramMgr.getCFA(rIspCamInfo.rIspIntInfo.u2CfaLowerIdx), // CFA settings for lower ISO
                          cfa);  // Output
             }
         }
         else {
             if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                rIspCamInfo.rIspIntInfo.u4CfaUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                rIspCamInfo.rIspIntInfo.u4CfaLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                rIspCamInfo.rIspIntInfo.u2CfaUpperIdx = m_IspNvramMgr.getIdx_CFA();
                rIspCamInfo.rIspIntInfo.u2CfaLowerIdx = m_IspNvramMgr.getIdx_CFA()-1;

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, rIspCamInfo.u4ISOValue,
                          rIspCamInfo.rIspIntInfo.u4CfaUpperISO,
                          rIspCamInfo.rIspIntInfo.u4CfaLowerISO,
                          rIspCamInfo.rIspIntInfo.u2CfaUpperIdx,
                          rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);

                SmoothCFA(rIspCamInfo.u4ISOValue,  // Real ISO
                rIspCamInfo.rIspIntInfo.u4CfaUpperISO, // Upper ISO
                rIspCamInfo.rIspIntInfo.u4CfaLowerISO, // Lower ISO
                m_IspNvramMgr.getCFA(rIspCamInfo.rIspIntInfo.u2CfaUpperIdx), // CFA settings for upper ISO
                m_IspNvramMgr.getCFA(rIspCamInfo.rIspIntInfo.u2CfaLowerIdx), // CFA settings for lower ISO
                cfa);  // Output
             }
         }
    }
#endif
    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_CFA(rIspCamInfo, m_IspNvramMgr, cfa);
    }

    // FG mode protection
    if (fgRPGEnable && ((rIspCamInfo.eIspProfile == EIspProfile_Preview)      ||
                          (rIspCamInfo.eIspProfile == EIspProfile_Video)        ||
                          //(rIspCamInfo.eIspProfile == EIspProfile_N3D_Preview)  ||
                          //(rIspCamInfo.eIspProfile == EIspProfile_N3D_Video)    ||
                          (rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
                          (rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
                          (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
                          (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video))) {
        cfa.byp.bits.DM_FG_MODE = 1;
    }
    else {
        cfa.byp.bits.DM_FG_MODE = 0;
    }


    if ((!ISP_MGR_CFA_T::getInstance(m_eSensorDev).isCCTEnable()) || (bDisable)) // CCT usage: fix CFA index
    {
        cfa = m_IspNvramMgr.getCFA(NVRAM_CFA_DISABLE_IDX);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, cfa );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] rIdx %p", __FUNCTION__, &rIdx);
    MBOOL bDisable = propertyGet("vendor.isp.ccm.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_CCM_T::getInstance(m_eSensorDev).isCCTEnable())            ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2))
    {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else if(rIspCamInfo.i4P2InImgFmt == 5)
    {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        ISP_NVRAM_CCM_T ccm = m_pCcmMgr->getCCM();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ccm );
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, rIspCamInfo.rMtkCCM);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.ggm.disable", 0);

    //ISP_NVRAM_GGM_T ggm;

    if ((bDisable) ||
        (!ISP_MGR_GGM_T::getInstance(m_eSensorDev).isCCTEnable())            ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, rIspCamInfo.rMtkGGM);
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.anr.disable", 0);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(rIspCamInfo);

    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)) {

        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANREnable(MFALSE);
    }
    else if ((rIspCamInfo.eIspProfile == EIspProfile_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_PureRAW_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {

        if (rIspCamInfo.u4ISOValue >= m_u4SwnrEncEnableIsoThreshold) {
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENCEnable(MTRUE);
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENYEnable(MTRUE);
        }
        else {
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENCEnable(MFALSE);
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR_ENYEnable(MTRUE);
        }

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_ANR();
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR(u2Idx);
#if 0
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo)) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR()+1;
                    rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(rIspCamInfo.u4ISOValue,  // Real ISO
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
             else {
                 if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                    rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR();
                    rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(rIspCamInfo.u4ISOValue,  // Real ISO
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
         }
#endif
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
        }

        // set ANR_LCE_LINK
//#warning "SWNR linker error"
#if 0
        SwNRParam::getInstance(m_i4SensorIdx)->setANR_LCE_LINK(static_cast<MBOOL>(anr.con1.bits.ANR_LCE_LINK));
        m_pLscMgr->setSwNr();
#endif
        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
    }
    else {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANREnable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_ANR();
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR(u2Idx);
#if 0
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo)) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR()+1;
                    rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(rIspCamInfo.u4ISOValue,  // Real ISO
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
             else {
                 if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                    rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR();
                    rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(rIspCamInfo.u4ISOValue,  // Real ISO
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
         }
#endif
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
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
    MBOOL bDisable = propertyGet("vendor.isp.ccr.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTCCREnable()) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)       ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_NBC_T::getInstance(m_eSensorDev).setCCREnable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_CCR();
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR(u2Idx);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCR(rIspCamInfo, m_IspNvramMgr, ccr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ccr );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PCA(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] rIdx %p", __FUNCTION__, &rIdx);
    MBOOL bDisable = propertyGet("vendor.isp.pca.disable", 0);

    //MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

    if (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
        m_ePCAMode = EPCAMode_360BIN;
    }
    else {
        m_ePCAMode = EPCAMode_180BIN;
    }

    if ((bDisable) ||
        (!ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).isCCTEnable())||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MTRUE);
        //dynamic color
        ISP_NVRAM_PCA_LUT_T pca = m_rIspParam.ISPPca.PCA_LUT[0][rIspCamInfo.eIdx_CT];

        MBOOL bIsCapture = m_pIspTuningCustom->is_to_invoke_capture_in_color(rIspCamInfo);

        if (m_rIspParam.ISPPca.COLOR_Method &&
            (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo)))
        {// Smooth PCA
            SmoothPCA_TBL(  bIsCapture,
                            m_rIspParam.ISPPca.COLOR_CT_Env[rIspCamInfo.eIdx_CT_U],
                            m_rIspParam.ISPPca.COLOR_CT_Env[rIspCamInfo.eIdx_CT_L],
                            rIspCamInfo.eIdx_CT_U, rIspCamInfo.eIdx_CT_L,
                            m_rIspParam.ISPPca.PCA_LUT,
                            m_rIspParam.ISPPca.rPcaParam,
                            rIspCamInfo,
                            pca,
                            0,
                            m_bSmoothPCA_FirstTimeBoot,
                            m_SmoothPCA_Prv);
        }

        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(reinterpret_cast<MUINT32*>(&pca.lut[0]));
        m_pPcaMgr->loadConfig();
    }

#if 0
    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

    if (m_ePCAMode == EPCAMode_360BIN) { // for VFB
#warning "VFB linker error"
#if 0
        MY_LOG_IF(m_bDebugEnable,"%s: loading vFB PCA (0x%x) ...", __FUNCTION__, halVFBTuning::getInstance().mHalVFBTuningGetPCA());
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(halVFBTuning::getInstance().mHalVFBTuningGetPCA());
#endif
    }
    else {
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
#endif
    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EE(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.ee.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_SEEE_T::getInstance(m_eSensorDev).isCCTEnable())           ||
        (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off)          ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_EE();
        ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE(u2Idx);
#if 0
        if (m_IspInterpCtrl
            &&m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo)) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2EEUpperIdx = m_IspNvramMgr.getIdx_EE()+1;
                    rIspCamInfo.rIspIntInfo.u2EELowerIdx = m_IspNvramMgr.getIdx_EE();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(rIspCamInfo.u4ISOValue,  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
             else {
                 if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                    rIspCamInfo.rIspIntInfo.u2EEUpperIdx = m_IspNvramMgr.getIdx_EE();
                    rIspCamInfo.rIspIntInfo.u2EELowerIdx = m_IspNvramMgr.getIdx_EE()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(rIspCamInfo.u4ISOValue,  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
         }
#endif
        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_EE(rIspCamInfo, m_IspNvramMgr, ee);

            if (rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
            {
                // User setting
                m_pIspTuningCustom->userSetting_EE(rIspCamInfo, rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);
            }
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ee);
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
    MY_LOG_IF(m_bDebugEnable, "[%s] rIdx %p", __FUNCTION__, &rIdx);
    MBOOL bDisable = propertyGet("vendor.isp.g2c.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_G2C_T::getInstance(m_eSensorDev).isCCTEnable())              ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta) {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MTRUE);
        //}

        ISP_NVRAM_G2C_T g2c;
        ISP_NVRAM_G2C_SHADE_T g2c_shade;
        ISP_NVRAM_SE_T se;
        ISP_NVRAM_GGM_T ggm;

        // Get ISP HW buffer
        getIspHWBuf(m_eSensorDev, g2c);
        getIspHWBuf(m_eSensorDev, g2c_shade);
        getIspHWBuf(m_eSensorDev, se);
//return to GGM setting        getIspHWBuf(m_eSensorDev, ggm);

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->userSetting_EFFECT(rIspCamInfo, rIspCamInfo.eIdx_Effect, rIspCamInfo.rIspUsrSelectLevel, g2c, g2c_shade, se, ggm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, g2c);
        putIspHWBuf(m_eSensorDev, g2c_shade);
        putIspHWBuf(m_eSensorDev, se);
//return to GGM setting        putIspHWBuf(m_eSensorDev, ggm);
    }

    return  MTRUE;
}
#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0

    // Get default NVRAM parameter
    MUINT16 u2Idx = rIdx.getIdx_NR3D();
    ISP_NVRAM_NR3D_T nr3d = m_IspNvramMgr.getNR3D(u2Idx);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NR3D(rIspCamInfo, m_IspNvramMgr, nr3d);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, nr3d );
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MFB
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MFB(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0

    // Get default NVRAM parameter
    MUINT16 u2Idx = rIdx.getIdx_MFB();
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB(u2Idx);

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MFB(rIspCamInfo, m_IspNvramMgr, mfb);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, mfb );
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Mixer3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_MIXER3(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    // Get default NVRAM parameter
    ISP_NVRAM_MIXER3_T mixer3 = m_rIspParam.ISPMfbMixer;

    // Since the original design can't apply mixer3 by ISO, and the nvram of mfb mix has never been used.
    // We steal the nvram of mfb mix and apply it to mixer3.
    // Get default NVRAM parameter
    MUINT16 u2Idx = rIdx.getIdx_MFB();
    ISP_NVRAM_MFB_T mfb = m_IspNvramMgr.getMFB(u2Idx);
    ISP_NVRAM_MIXER3_T mixer3;
    //
    mixer3.ctrl_0.bits.MIX3_WT_SEL = 1;
    mixer3.ctrl_0.bits.MIX3_B0 = mfb.ll_con6.bits.BLD_LL_MX_B0;
    mixer3.ctrl_0.bits.MIX3_B1 = mfb.ll_con6.bits.BLD_LL_MX_B1;
    mixer3.ctrl_0.bits.MIX3_DT = mfb.ll_con5.bits.BLD_LL_MX_DT;
    //
    mixer3.ctrl_1.bits.MIX3_M0 = mfb.ll_con5.bits.BLD_LL_MX_M0;
    mixer3.ctrl_1.bits.MIX3_M1 = mfb.ll_con5.bits.BLD_LL_MX_M1;


    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_MIXER3(rIspCamInfo, m_IspNvramMgr, mixer3);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, mixer3 );
#endif

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LCE(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0   //this chip doesn't have LCE
    // Get default NVRAM parameter
    ISP_NVRAM_LCE_T lce = m_IspNvramMgr.getLCE();

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_LCE(m_rIspCamInfo, m_IspNvramMgr, lce);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, lce);

#endif

    return  MTRUE;
}
#endif
