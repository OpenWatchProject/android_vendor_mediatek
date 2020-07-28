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
               //&&  ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg);
				;
	}

    fgRet = ISP_MGR_NSL2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_UDM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NBC2_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_PCA_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_LCE_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        &&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
        ;

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
        &&  prepareHw_PerFrame_UDM(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_CCM(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_GGM(rIspInfo.rCamInfo)
        //&&  prepareHw_PerFrame_GGM(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_NSL2(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_ANR(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_ANR2(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_CCR(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_PCA(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_EE(const_cast<RAWIspCamInfo&>(rIspInfo.rCamInfo), rIdx)
        &&  prepareHw_PerFrame_EFFECT(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_LSC_2(rIspInfo)
        &&  prepareHw_PerFrame_NR3D(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_MFB(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_MIXER3(rIspInfo.rCamInfo, rIdx)
        &&  prepareHw_PerFrame_LCE(rIspInfo.rCamInfo, rIdx)
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
        &&  prepareHw_PerFrame_UDM()
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

    prepareHw_PerFrame_UDM(temp, rIdx);
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
        ISP_NVRAM_OBC_T obc = m_IspNvramMgr.getOBC();

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
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTCTEnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
        (m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
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
                    m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx = m_IspNvramMgr.getIdx_PDC()-1;

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
prepareHw_PerFrame_NSL2()
{
    // Get default NVRAM parameter
    ISP_NVRAM_NSL2_T nsl2;

    getIspHWBuf(m_eSensorDev, nsl2 );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NSL2(m_rIspCamInfo, m_IspNvramMgr, nsl2);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, nsl2 );

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
prepareHw_PerFrame_UDM()
{
		MBOOL bDisable = propertyGet("vendor.isp.udm.disable", 0);

		if ((m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) 	   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)		   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off) 	   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)		   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc) 			   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)	   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)	   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)		   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)		   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)		   ||
			(m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
		   )
		{
			return MTRUE;
		}

		// Get default NVRAM parameter
		ISP_NVRAM_UDM_T udm = m_IspNvramMgr.getUDM();

		if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl) {
			if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
				 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
					m_rIspCamInfo.rIspIntInfo.u4UdmUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
					m_rIspCamInfo.rIspIntInfo.u4UdmLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
					m_rIspCamInfo.rIspIntInfo.u2UdmUpperIdx = m_IspNvramMgr.getIdx_UDM()+1;
					m_rIspCamInfo.rIspIntInfo.u2UdmLowerIdx = m_IspNvramMgr.getIdx_UDM();

					MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
							  __FUNCTION__, m_rIspCamInfo.u4ISOValue,
							  m_rIspCamInfo.rIspIntInfo.u4UdmUpperISO,
							  m_rIspCamInfo.rIspIntInfo.u4UdmLowerISO,
							  m_rIspCamInfo.rIspIntInfo.u2UdmUpperIdx,
							  m_rIspCamInfo.rIspIntInfo.u2UdmLowerIdx);

					SmoothUDM(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
							  m_rIspCamInfo.rIspIntInfo.u4UdmUpperISO, // Upper ISO
							  m_rIspCamInfo.rIspIntInfo.u4UdmLowerISO, // Lower ISO
							  m_IspNvramMgr.getUDM(m_rIspCamInfo.rIspIntInfo.u2UdmUpperIdx), // CFA settings for upper ISO
							  m_IspNvramMgr.getUDM(m_rIspCamInfo.rIspIntInfo.u2UdmLowerIdx), // CFA settings for lower ISO
							  udm);  // Output
				 }
			 }
			 else {
				 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
					m_rIspCamInfo.rIspIntInfo.u4UdmUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
					m_rIspCamInfo.rIspIntInfo.u4UdmLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
					m_rIspCamInfo.rIspIntInfo.u2UdmUpperIdx = m_IspNvramMgr.getIdx_UDM();
					m_rIspCamInfo.rIspIntInfo.u2UdmLowerIdx = m_IspNvramMgr.getIdx_UDM()-1;

					MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
							  __FUNCTION__, m_rIspCamInfo.u4ISOValue,
							  m_rIspCamInfo.rIspIntInfo.u4UdmUpperISO,
							  m_rIspCamInfo.rIspIntInfo.u4UdmLowerISO,
							  m_rIspCamInfo.rIspIntInfo.u2UdmUpperIdx,
							  m_rIspCamInfo.rIspIntInfo.u2UdmLowerIdx);

					SmoothUDM(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
					m_rIspCamInfo.rIspIntInfo.u4UdmUpperISO, // Upper ISO
					m_rIspCamInfo.rIspIntInfo.u4UdmLowerISO, // Lower ISO
					m_IspNvramMgr.getUDM(m_rIspCamInfo.rIspIntInfo.u2UdmUpperIdx), // UDM settings for upper ISO
					m_IspNvramMgr.getUDM(m_rIspCamInfo.rIspIntInfo.u2UdmLowerIdx), // UDM settings for lower ISO
					udm);  // Output
				 }
			 }
		}
		// Invoke callback for customers to modify.
		if	( isDynamicTuning() )
		{	//	Dynamic Tuning: Enable
			m_pIspTuningCustom->refine_UDM(m_rIspCamInfo, m_IspNvramMgr, udm);
		}

		// FG mode protection
		if (isRPGEnable() && ((m_rIspCamInfo.eIspProfile == EIspProfile_Preview)	  ||
							  (m_rIspCamInfo.eIspProfile == EIspProfile_Video)		  ||
							  (m_rIspCamInfo.eIspProfile == EIspProfile_N3D_Preview)  ||
							  (m_rIspCamInfo.eIspProfile == EIspProfile_N3D_Video)	  ||
							  (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
							  (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
							  (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
							  (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video))) {
			udm.dsb.bits.UDM_FL_MODE = 1;
			udm.dsb.bits.UDM_SC_RAT = 16;
			udm.dsb.bits.UDM_SL_RAT = 16;
		}
		else {
			udm.dsb.bits.UDM_FL_MODE = 0;	  //cfa.byp.bit 	DM_FG_MODE
		}


		if ((!ISP_MGR_UDM_T::getInstance(m_eSensorDev).isCCTEnable()) || (bDisable)) // CCT usage: fix CFA index
		{
			udm = m_IspNvramMgr.getUDM(NVRAM_UDM_DISABLE_IDX);
		}

		// Load it to ISP manager buffer.
		putIspHWBuf(m_eSensorDev, udm );

		return	MTRUE;
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
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        ISP_NVRAM_CCM_T ccm;

        if (m_rIspCamInfo.fgRWBSensor)
        {
            // Need to generate ISP_CCM, GPU_CCM, MDP_CCM
            //ccm is ISP_CCM
           // ccm = m_pCcmMgr->getCCM(E_ISP_CCM);
            //ISP_MGR_CCM_T::getInstance(m_eSensorDev).putMdpCCM(m_pCcmMgr->getCCM(E_MDP_CCM));
        }
        else //normal sensor
        {
        // Get default NVRAM parameter
            ccm = m_pCcmMgr->getCCM();
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCM(m_rIspCamInfo, m_IspNvramMgr, ccm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ccm );
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

    #if 1
    if ((bDisable) ||
         (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANR1Enable()) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR))
    #else
    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR))
    #endif
    {

        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR1Enable(MFALSE);
    }
    else {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR1Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = m_IspNvramMgr.getIdx_ANR()+1;
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR();

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
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
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
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

    bDisable = propertyGet("vendor.isp.anr_tbl.disable", 0);
    ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANRTBLEnable(!bDisable);

    //ANR_ISO_IDX
    MBOOL fgNeedInterp;
    MUINT32 UpperIso, LowerIso;

    m_rIspCamInfo.eIdx_ANR_TBL_ISO = m_pIspTuningCustom->evaluate_ANR_TBL_ISO_index(m_rIspCamInfo, fgNeedInterp, UpperIso, LowerIso);

    if(fgNeedInterp){
        ISP_NVRAM_ANR_LUT_T* upperTbl;
        ISP_NVRAM_ANR_LUT_T* lowerTbl;
        ISP_NVRAM_ANR_LUT_T outTbl;
        lowerTbl = reinterpret_cast<ISP_NVRAM_ANR_LUT_T*>(&(m_rIspParam.ANR_TBL[m_rIspCamInfo.eIdx_ANR_TBL_CT][m_rIspCamInfo.eIdx_ANR_TBL_ISO][0]));
        upperTbl = reinterpret_cast<ISP_NVRAM_ANR_LUT_T*>(&(m_rIspParam.ANR_TBL[m_rIspCamInfo.eIdx_ANR_TBL_CT][m_rIspCamInfo.eIdx_ANR_TBL_ISO + 1][0]));
        SmoothANR_TBL(m_rIspCamInfo.u4ISOValue, UpperIso, LowerIso,
                      *upperTbl, *lowerTbl, outTbl);
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).putANR_LUT(outTbl);
    }
    else{
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).putANR_TBL(m_rIspParam.ANR_TBL[m_rIspCamInfo.eIdx_ANR_TBL_CT][m_rIspCamInfo.eIdx_ANR_TBL_ISO]);
    }

    return  MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_ANR2()
{
	MBOOL bDisable = propertyGet("vendor.isp.anr2.disable", 0);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(m_rIspCamInfo);

    #if 1
    if ((bDisable) ||
         (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTANR2Enable()) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc))
    #else
    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR))
    #endif
    {

        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2Enable(MFALSE);
    }
    else {
        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2Enable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_ANR2_T anr2 = m_IspNvramMgr.getANR2();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = m_IspNvramMgr.getIdx_ANR2()+1;
                    m_rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = m_IspNvramMgr.getIdx_ANR2();

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR2(m_rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR2(m_rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx), // ANR settings for lower ISO
                              anr2);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = m_IspNvramMgr.getIdx_ANR2();
                    m_rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = m_IspNvramMgr.getIdx_ANR2()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR2(m_rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR2(m_rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx), // ANR settings for lower ISO
                              anr2);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR2(m_rIspCamInfo, m_IspNvramMgr, anr2);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr2 );
    }

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
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTCCREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)       ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)  ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)       ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)
       )
    {
        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setCCREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR();


        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl)
        {
                if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                     if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                        m_rIspCamInfo.rIspIntInfo.u4CcrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                        m_rIspCamInfo.rIspIntInfo.u4CcrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                        m_rIspCamInfo.rIspIntInfo.u2CcrUpperIdx = m_IspNvramMgr.getIdx_CCR()+1;
                        m_rIspCamInfo.rIspIntInfo.u2CcrLowerIdx = m_IspNvramMgr.getIdx_CCR();

                        MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                                  __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                                  m_rIspCamInfo.rIspIntInfo.u4CcrUpperISO,
                                  m_rIspCamInfo.rIspIntInfo.u4CcrLowerISO,
                                  m_rIspCamInfo.rIspIntInfo.u2CcrUpperIdx,
                                  m_rIspCamInfo.rIspIntInfo.u2CcrLowerIdx);

                        SmoothCCR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                                  m_rIspCamInfo.rIspIntInfo.u4CcrUpperISO, // Upper ISO
                                  m_rIspCamInfo.rIspIntInfo.u4CcrLowerISO, // Lower ISO
                                  m_IspNvramMgr.getCCR(m_rIspCamInfo.rIspIntInfo.u2CcrUpperIdx), // CFA settings for upper ISO
                                  m_IspNvramMgr.getCCR(m_rIspCamInfo.rIspIntInfo.u2CcrLowerIdx), // CFA settings for lower ISO
                                  ccr);  // Output
                     }
                 }
                 else {
                     if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                        m_rIspCamInfo.rIspIntInfo.u4CcrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                        m_rIspCamInfo.rIspIntInfo.u4CcrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                        m_rIspCamInfo.rIspIntInfo.u2CcrUpperIdx = m_IspNvramMgr.getIdx_CCR();
                        m_rIspCamInfo.rIspIntInfo.u2CcrLowerIdx = m_IspNvramMgr.getIdx_CCR()-1;

                        MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                                  __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                                  m_rIspCamInfo.rIspIntInfo.u4CcrUpperISO,
                                  m_rIspCamInfo.rIspIntInfo.u4CcrLowerISO,
                                  m_rIspCamInfo.rIspIntInfo.u2CcrUpperIdx,
                                  m_rIspCamInfo.rIspIntInfo.u2CcrLowerIdx);

                        SmoothCCR(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                        m_rIspCamInfo.rIspIntInfo.u4CcrUpperISO, // Upper ISO
                        m_rIspCamInfo.rIspIntInfo.u4CcrLowerISO, // Lower ISO
                        m_IspNvramMgr.getCCR(m_rIspCamInfo.rIspIntInfo.u2CcrUpperIdx), // CCR settings for upper ISO
                        m_IspNvramMgr.getCCR(m_rIspCamInfo.rIspIntInfo.u2CcrLowerIdx), // CCR settings for lower ISO
                        ccr);  // Output
                     }
                 }
            }

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
prepareHw_PerFrame_BOK()
{
	MBOOL bDisable = propertyGet("vendor.isp.bok.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTCCREnable())||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)){
        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setBOKEnable(MFALSE);   //0724forbuild
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
         ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setBOKEnable(MTRUE);   //0724forbuild

        // Get default NVRAM parameter
        ISP_NVRAM_BOK_T bok = m_IspNvramMgr.getBOK();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BOK(m_rIspCamInfo, m_IspNvramMgr, bok);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, bok );
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

    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

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
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2))
    {
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MTRUE);
    }


    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

   /* if (m_ePCAMode == EPCAMode_360BIN) { // for VFB
        MY_LOG_IF(m_bDebugEnable,"%s: loading vFB PCA (0x%x) ...", __FUNCTION__, halVFBTuning::getInstance().mHalVFBTuningGetPCA());
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(halVFBTuning::getInstance().mHalVFBTuningGetPCA());
    }
    else */
		{
        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl)
        {
            MBOOL fgNeedInter = MFALSE;
            MUINT32 u4CTlower, u4CTupper;
            m_rIspCamInfo.eIdx_PCA_LUT =
                m_pIspTuningCustom->evaluate_SmoothPCA_LUT_idx(m_rIspCamInfo, fgNeedInter, u4CTlower, u4CTupper);

            m_pPcaMgr->setIdx(static_cast<MUINT32>(m_rIspCamInfo.eIdx_PCA_LUT));

            if (fgNeedInter)
            {
                ISP_NVRAM_PCA_LUT_T* pca_lower;
                ISP_NVRAM_PCA_LUT_T* pca_upper;
                ISP_NVRAM_PCA_LUT_T pca_out;
                m_pPcaMgr->getLut( pca_lower, pca_upper);

                SmoothPCA_TBL(m_rIspCamInfo.rAWBInfo.i4CCT, u4CTupper, u4CTlower,
                              *pca_upper, *pca_lower, pca_out, m_rIspCamInfo.i4LightValue_x10,
                              (ISP_CUSTOM_PARA_PCA_T*)(m_pIspTuningCustom->get_SmoothPCA_para(m_eSensorDev)));
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
                m_pPcaMgr->setIdx(static_cast<MUINT32>(m_rIspCamInfo.eIdx_PCA_LUT));
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
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off)          ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_EE_Off)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    m_rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    m_rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx = m_IspNvramMgr.getIdx_EE()+1;
                    m_rIspCamInfo.rIspIntInfo.u2EELowerIdx = m_IspNvramMgr.getIdx_EE();

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx = m_IspNvramMgr.getIdx_EE();
                    m_rIspCamInfo.rIspIntInfo.u2EELowerIdx = m_IspNvramMgr.getIdx_EE()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(m_pIspTuningCustom->remap_ISO_value(m_rIspCamInfo.u4ISOValue),  // Real ISO
                             m_rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             m_rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(m_rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
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
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta) {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MTRUE);
        //}

        //ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MTRUE);

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
            m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfo, m_rIspCamInfo.eIdx_Effect,m_rIspCamInfo.rIspUsrSelectLevel, g2c, g2c_shade, se, ggm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, g2c);
        putIspHWBuf(m_eSensorDev, g2c_shade);
        putIspHWBuf(m_eSensorDev, se);
//return to GGM setting        putIspHWBuf(m_eSensorDev, ggm);
    }

    return  MTRUE;
}
#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D()
{
#if 1
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
#if 1
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
#else
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
#endif

		// Invoke callback for customers to modify.
		if	( isDynamicTuning() )
		{	//	Dynamic Tuning: Enable
			m_pIspTuningCustom->refine_MIXER3(m_rIspCamInfo, m_IspNvramMgr, mixer3);
		}

		// Load it to ISP manager buffer
		putIspHWBuf(m_eSensorDev, mixer3 );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LCE()
{
#if 1   //this chip doesn't have LCE
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
                    rIspCamInfo.rIspIntInfo.u2DbsUpperIdx = rIdx.getIdx_DBS()+1;
                    rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = rIdx.getIdx_DBS();

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
                    rIspCamInfo.rIspIntInfo.u2DbsUpperIdx = rIdx.getIdx_DBS();
                    if (rIdx.getIdx_DBS() != 0){
                        rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = rIdx.getIdx_DBS()-1;
                    } else {
                        rIspCamInfo.rIspIntInfo.u2DbsLowerIdx = rIdx.getIdx_DBS();
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
                    rIspCamInfo.rIspIntInfo.u2BpcUpperIdx = rIdx.getIdx_BPC()+1;
                    rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = rIdx.getIdx_BPC();

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
                    rIspCamInfo.rIspIntInfo.u2BpcUpperIdx = rIdx.getIdx_BPC();
                    if (rIdx.getIdx_BPC() != 0){
                        rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = rIdx.getIdx_BPC()-1;
                    } else {
                        rIspCamInfo.rIspIntInfo.u2BpcLowerIdx = rIdx.getIdx_BPC();
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



MBOOL
Paramctrl::
prepareHw_PerFrame_NSL2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
     // Get default NVRAM parameter
    ISP_NVRAM_NSL2_T nsl2;

    getIspHWBuf(m_eSensorDev, nsl2 );

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_NSL2(rIspCamInfo, m_IspNvramMgr, nsl2);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, nsl2 );

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
    MBOOL bDisable = propertyGet("vendor.isp.pgn.disable", 0);
	MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;

    MY_LOG_IF(m_bDebugEnable,"%s(): fgRPGEnable(%d)", __FUNCTION__, fgRPGEnable);

    // Get default NVRAM parameter
    ISP_NVRAM_PGN_T pgn;

    AWB_GAIN_T rCurrentAWBGain = rIspCamInfo.rAWBInfo.rCurrentAWBGain;

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
prepareHw_PerFrame_UDM(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
		MBOOL bDisable = propertyGet("vendor.isp.udm.disable", 0);
		MBOOL fgRPGEnable = rIspCamInfo.fgRPGEnable;

		if ((rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) 	   ||
			(rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
			(rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)		   ||
			(rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off) 	   ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)		   ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
			(rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc) 			   ||
			(rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)	   ||
			(rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)	   ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
			(rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)		   ||
			(rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)		   ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)		   ||
			(rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
		   )
		{
			return MTRUE;
		}

		// Get default NVRAM parameter
		MUINT16 u2Idx = rIdx.getIdx_UDM();
		ISP_NVRAM_UDM_T udm = m_IspNvramMgr.getUDM(u2Idx);


		if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
			if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
				 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
					rIspCamInfo.rIspIntInfo.u4UdmUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
					rIspCamInfo.rIspIntInfo.u4UdmLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
					rIspCamInfo.rIspIntInfo.u2UdmUpperIdx = rIdx.getIdx_UDM()+1;
					rIspCamInfo.rIspIntInfo.u2UdmLowerIdx = rIdx.getIdx_UDM();

					MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
							  __FUNCTION__, rIspCamInfo.u4ISOValue,
							  rIspCamInfo.rIspIntInfo.u4UdmUpperISO,
							  rIspCamInfo.rIspIntInfo.u4UdmLowerISO,
							  rIspCamInfo.rIspIntInfo.u2UdmUpperIdx,
							  rIspCamInfo.rIspIntInfo.u2UdmLowerIdx);

					SmoothUDM(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
							  rIspCamInfo.rIspIntInfo.u4UdmUpperISO, // Upper ISO
							  rIspCamInfo.rIspIntInfo.u4UdmLowerISO, // Lower ISO
							  m_IspNvramMgr.getUDM(rIspCamInfo.rIspIntInfo.u2UdmUpperIdx), // CFA settings for upper ISO
							  m_IspNvramMgr.getUDM(rIspCamInfo.rIspIntInfo.u2UdmLowerIdx), // CFA settings for lower ISO
							  udm);  // Output
				 }
			 }
			 else {
				 if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
					rIspCamInfo.rIspIntInfo.u4UdmUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
					rIspCamInfo.rIspIntInfo.u4UdmLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
					rIspCamInfo.rIspIntInfo.u2UdmUpperIdx = rIdx.getIdx_UDM();
					rIspCamInfo.rIspIntInfo.u2UdmLowerIdx = rIdx.getIdx_UDM()-1;

					MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
							  __FUNCTION__, rIspCamInfo.u4ISOValue,
							  rIspCamInfo.rIspIntInfo.u4UdmUpperISO,
							  rIspCamInfo.rIspIntInfo.u4UdmLowerISO,
							  rIspCamInfo.rIspIntInfo.u2UdmUpperIdx,
							  rIspCamInfo.rIspIntInfo.u2UdmLowerIdx);

					SmoothUDM(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
					rIspCamInfo.rIspIntInfo.u4UdmUpperISO, // Upper ISO
					rIspCamInfo.rIspIntInfo.u4UdmLowerISO, // Lower ISO
					m_IspNvramMgr.getUDM(rIspCamInfo.rIspIntInfo.u2UdmUpperIdx), // UDM settings for upper ISO
					m_IspNvramMgr.getUDM(rIspCamInfo.rIspIntInfo.u2UdmLowerIdx), // UDM settings for lower ISO
					udm);  // Output
				 }
			 }
		}

		// Invoke callback for customers to modify.
		if	( isDynamicTuning() )
		{	//	Dynamic Tuning: Enable
			m_pIspTuningCustom->refine_UDM(rIspCamInfo, m_IspNvramMgr, udm);
		}

		// FG mode protection
		if (fgRPGEnable && ((rIspCamInfo.eIspProfile == EIspProfile_Preview)	  ||
							  (rIspCamInfo.eIspProfile == EIspProfile_Video)		  ||
							  (rIspCamInfo.eIspProfile == EIspProfile_N3D_Preview)  ||
							  (rIspCamInfo.eIspProfile == EIspProfile_N3D_Video)	  ||
							  (rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
							  (rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
							  (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
							  (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video))) {
			udm.dsb.bits.UDM_FL_MODE = 1;
			udm.dsb.bits.UDM_SC_RAT = 16;
			udm.dsb.bits.UDM_SL_RAT = 16;
		}
		else {
			udm.dsb.bits.UDM_FL_MODE = 0;	  //cfa.byp.bit 	DM_FG_MODE
		}


		if ((!ISP_MGR_UDM_T::getInstance(m_eSensorDev).isCCTEnable()) || (bDisable)) // CCT usage: fix CFA index
		{
			udm = m_IspNvramMgr.getUDM(NVRAM_UDM_DISABLE_IDX);
		}

		// Load it to ISP manager buffer.
		putIspHWBuf(m_eSensorDev, udm );

		return	MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.ccm.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_CCM_T::getInstance(m_eSensorDev).isCCTEnable())            ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        ISP_NVRAM_CCM_T ccm;

        if (rIspCamInfo.fgRWBSensor)
        {
            // Need to generate ISP_CCM, GPU_CCM, MDP_CCM
            //ccm is ISP_CCM
           // ccm = m_pCcmMgr->getCCM(E_ISP_CCM);
           // ISP_MGR_CCM_T::getInstance(m_eSensorDev).putMdpCCM(m_pCcmMgr->getCCM(E_MDP_CCM));
        }
        else //normal sensor
        {
        // Get default NVRAM parameter
            ccm = m_pCcmMgr->getCCM();
        }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_CCM(rIspCamInfo, m_IspNvramMgr, ccm);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, ccm );
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

    ISP_NVRAM_GGM_T ggm;

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
	MY_LOG_IF(m_bDebugEnable, "rIspCamInfo.eNRMode=%d",rIspCamInfo.eNRMode);
    #if 1
    if ((bDisable) ||
         (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANR1Enable()) ||
         (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
         (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
         (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
         (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
         (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR))
    #else
    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR))
    #endif
    {

        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR1Enable(MFALSE);
    }
    else {
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR1Enable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_ANR();
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR(u2Idx);

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = rIdx.getIdx_ANR()+1;
                    rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = rIdx.getIdx_ANR();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
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
                    rIspCamInfo.rIspIntInfo.u2AnrUpperIdx = rIdx.getIdx_ANR();
                    rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = rIdx.getIdx_ANR()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2AnrUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

                    SmoothANR(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrUpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR(rIspCamInfo.rIspIntInfo.u2AnrLowerIdx), // ANR settings for lower ISO
                              anr);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
    }

    char value2[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.anr_tbl.disable", value2, "0"); // 0: enable, 1: disable
    ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANRTBLEnable(!(atoi(value2)));

    //ANR_ISO_IDX
    MBOOL fgNeedInterp;
    MUINT32 UpperIso, LowerIso;

    rIspCamInfo.eIdx_ANR_TBL_ISO = m_pIspTuningCustom->evaluate_ANR_TBL_ISO_index(rIspCamInfo, fgNeedInterp, UpperIso, LowerIso);

    if(fgNeedInterp){
        ISP_NVRAM_ANR_LUT_T* upperTbl;
        ISP_NVRAM_ANR_LUT_T* lowerTbl;
        ISP_NVRAM_ANR_LUT_T outTbl;
        lowerTbl = reinterpret_cast<ISP_NVRAM_ANR_LUT_T*>(&(m_rIspParam.ANR_TBL[rIspCamInfo.eIdx_ANR_TBL_CT][rIspCamInfo.eIdx_ANR_TBL_ISO][0]));
        upperTbl = reinterpret_cast<ISP_NVRAM_ANR_LUT_T*>(&(m_rIspParam.ANR_TBL[rIspCamInfo.eIdx_ANR_TBL_CT][rIspCamInfo.eIdx_ANR_TBL_ISO + 1][0]));
        SmoothANR_TBL(rIspCamInfo.u4ISOValue, UpperIso, LowerIso,
                      *upperTbl, *lowerTbl, outTbl);
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).putANR_LUT(outTbl);
    }
    else{
        ISP_MGR_NBC_T::getInstance(m_eSensorDev).putANR_TBL(m_rIspParam.ANR_TBL[rIspCamInfo.eIdx_ANR_TBL_CT][rIspCamInfo.eIdx_ANR_TBL_ISO]);
    }

    return  MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR2(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.anr2.disable", 0);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(rIspCamInfo);
	MY_LOG_IF(m_bDebugEnable, "rIspCamInfo.eNRMode=%d",rIspCamInfo.eNRMode);
    #if 1
    if ((bDisable) ||
         (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTANR2Enable()) ||
         (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
         (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
         (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
         (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
         (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR) ||
         (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc))
    #else
    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR))
    #endif
    {

        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2Enable(MFALSE);
    }
    else {
        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2Enable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_ANR2();
        ISP_NVRAM_ANR2_T anr2 = m_IspNvramMgr.getANR2(u2Idx);

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = rIdx.getIdx_ANR2()+1;
                    rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = rIdx.getIdx_ANR2();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR2(rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR2(rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx), // ANR settings for lower ISO
                              anr2);  // Output
                 }
             }
             else {
                 if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                    rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = rIdx.getIdx_ANR2();
                    rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = rIdx.getIdx_ANR2()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO, // Upper ISO
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO, // Lower ISO
                              m_IspNvramMgr.getANR2(rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx), // ANR settings for upper ISO
                              m_IspNvramMgr.getANR2(rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx), // ANR settings for lower ISO
                              anr2);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR2(rIspCamInfo, m_IspNvramMgr, anr2);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr2 );
    }

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.ccr.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTCCREnable()) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)       ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)  ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)       ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)
       )
    {
        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setCCREnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR();


        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl)
        {
                if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                     if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                        rIspCamInfo.rIspIntInfo.u4CcrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                        rIspCamInfo.rIspIntInfo.u4CcrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                        rIspCamInfo.rIspIntInfo.u2CcrUpperIdx = rIdx.getIdx_CCR()+1;
                        rIspCamInfo.rIspIntInfo.u2CcrLowerIdx = rIdx.getIdx_CCR();

                        MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                                  __FUNCTION__, rIspCamInfo.u4ISOValue,
                                  rIspCamInfo.rIspIntInfo.u4CcrUpperISO,
                                  rIspCamInfo.rIspIntInfo.u4CcrLowerISO,
                                  rIspCamInfo.rIspIntInfo.u2CcrUpperIdx,
                                  rIspCamInfo.rIspIntInfo.u2CcrLowerIdx);

                        SmoothCCR(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                                  rIspCamInfo.rIspIntInfo.u4CcrUpperISO, // Upper ISO
                                  rIspCamInfo.rIspIntInfo.u4CcrLowerISO, // Lower ISO
                                  m_IspNvramMgr.getCCR(rIspCamInfo.rIspIntInfo.u2CcrUpperIdx), // CFA settings for upper ISO
                                  m_IspNvramMgr.getCCR(rIspCamInfo.rIspIntInfo.u2CcrLowerIdx), // CFA settings for lower ISO
                                  ccr);  // Output
                     }
                 }
                 else {
                     if (rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                        rIspCamInfo.rIspIntInfo.u4CcrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                        rIspCamInfo.rIspIntInfo.u4CcrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO-1));
                        rIspCamInfo.rIspIntInfo.u2CcrUpperIdx = rIdx.getIdx_CCR();
                        rIspCamInfo.rIspIntInfo.u2CcrLowerIdx = rIdx.getIdx_CCR()-1;

                        MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                                  __FUNCTION__, rIspCamInfo.u4ISOValue,
                                  rIspCamInfo.rIspIntInfo.u4CcrUpperISO,
                                  rIspCamInfo.rIspIntInfo.u4CcrLowerISO,
                                  rIspCamInfo.rIspIntInfo.u2CcrUpperIdx,
                                  rIspCamInfo.rIspIntInfo.u2CcrLowerIdx);

                        SmoothCCR(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                        rIspCamInfo.rIspIntInfo.u4CcrUpperISO, // Upper ISO
                        rIspCamInfo.rIspIntInfo.u4CcrLowerISO, // Lower ISO
                        m_IspNvramMgr.getCCR(rIspCamInfo.rIspIntInfo.u2CcrUpperIdx), // CCR settings for upper ISO
                        m_IspNvramMgr.getCCR(rIspCamInfo.rIspIntInfo.u2CcrLowerIdx), // CCR settings for lower ISO
                        ccr);  // Output
                     }
                 }
            }

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
prepareHw_PerFrame_BOK(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.bok.disable", 0);

    if ((bDisable) ||
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTCCREnable())||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)){
        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setBOKEnable(MFALSE);   //0724forbuild
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
         ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setBOKEnable(MTRUE);   //0724forbuild

        // Get default NVRAM parameter
        ISP_NVRAM_BOK_T bok = m_IspNvramMgr.getBOK();

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_BOK(rIspCamInfo, m_IspNvramMgr, bok);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, bok );
    }

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PCA(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.pca.disable", 0);

    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

    if (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
        m_ePCAMode = EPCAMode_360BIN;
    }
    else {
        m_ePCAMode = EPCAMode_180BIN;
    }

    if ((bDisable) ||
        (!ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).isCCTEnable())  ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2))
    {
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).setEnable(MTRUE);
    }


    //  (1) Check to see whether PCA is enabled?
    if  (! m_pPcaMgr->isEnable())
    {
        return  MTRUE;
    }

   /* if (m_ePCAMode == EPCAMode_360BIN) { // for VFB
        MY_LOG_IF(m_bDebugEnable,"%s: loading vFB PCA (0x%x) ...", __FUNCTION__, halVFBTuning::getInstance().mHalVFBTuningGetPCA());
        ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).loadLut(halVFBTuning::getInstance().mHalVFBTuningGetPCA());
    }
    else */
		{
        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl)
        {
            MBOOL fgNeedInter = MFALSE;
            MUINT32 u4CTlower, u4CTupper;
            rIspCamInfo.eIdx_PCA_LUT =
                m_pIspTuningCustom->evaluate_SmoothPCA_LUT_idx(rIspCamInfo, fgNeedInter, u4CTlower, u4CTupper);

            m_pPcaMgr->setIdx(static_cast<MUINT32>(rIspCamInfo.eIdx_PCA_LUT));

            if (fgNeedInter)
            {
                ISP_NVRAM_PCA_LUT_T* pca_lower;
                ISP_NVRAM_PCA_LUT_T* pca_upper;
                ISP_NVRAM_PCA_LUT_T pca_out;
                m_pPcaMgr->getLut( pca_lower, pca_upper);

                SmoothPCA_TBL(rIspCamInfo.rAWBInfo.i4CCT, u4CTupper, u4CTlower,
                              *pca_upper, *pca_lower, pca_out, rIspCamInfo.i4LightValue_x10,
                              (ISP_CUSTOM_PARA_PCA_T*)(m_pIspTuningCustom->get_SmoothPCA_para(m_eSensorDev)));
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
prepareHw_PerFrame_EE(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MBOOL bDisable = propertyGet("vendor.isp.ee.disable", 0);
    MY_LOG_IF(m_bDebugEnable, "rIspCamInfo.eEdgeMode=%d,rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge = %d",rIspCamInfo.eEdgeMode,rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge);
    if ((bDisable) ||
        (!ISP_MGR_SEEE_T::getInstance(m_eSensorDev).isCCTEnable())             ||
        (rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off)          ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off)          ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_EE_Off)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_EE();
        ISP_NVRAM_EE_T ee = m_IspNvramMgr.getEE(u2Idx);

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2EEUpperIdx = rIdx.getIdx_EE()+1;
                    rIspCamInfo.rIspIntInfo.u2EELowerIdx = rIdx.getIdx_EE();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(m_pIspTuningCustom->remap_ISO_value(rIspCamInfo.u4ISOValue),  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    m_rIspCamInfo.rIspIntInfo.u4EEUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    m_rIspCamInfo.rIspIntInfo.u4EELowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx = rIdx.getIdx_EE();
                    m_rIspCamInfo.rIspIntInfo.u2EELowerIdx = rIdx.getIdx_EE()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              m_rIspCamInfo.rIspIntInfo.u4EEUpperISO,
                              m_rIspCamInfo.rIspIntInfo.u4EELowerISO,
                              m_rIspCamInfo.rIspIntInfo.u2EEUpperIdx,
                              m_rIspCamInfo.rIspIntInfo.u2EELowerIdx);

                    SmoothEE(rIspCamInfo.u4ISOValue,  // Real ISO
                             rIspCamInfo.rIspIntInfo.u4EEUpperISO, // Upper ISO
                             rIspCamInfo.rIspIntInfo.u4EELowerISO, // Lower ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EEUpperIdx), // EE settings for upper ISO
                             m_IspNvramMgr.getEE(rIspCamInfo.rIspIntInfo.u2EELowerIdx), // EE settings for lower ISO
                             ee);  // Output
                 }
             }
         }

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_EE(rIspCamInfo, m_IspNvramMgr, ee);

            /*if (rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge != MTK_CONTROL_ISP_EDGE_MIDDLE)
            {
                // User setting
                m_pIspTuningCustom->userSetting_EE(rIspCamInfo, rIspCamInfo.rIspUsrSelectLevel.eIdx_Edge, ee);
            }*/
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
    MBOOL bDisable = propertyGet("vendor.isp.g2c.disable", 0);

    MY_LOG_IF(m_bDebugEnable, "[%s] rIdx %p, ", __FUNCTION__, &rIdx);

    if ((bDisable) ||
        (!ISP_MGR_G2C_T::getInstance(m_eSensorDev).isCCTEnable())              ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)        ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)   ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR)    ||
        (rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)                ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_1)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_Capture_MultiPass_ANR_2)     ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1) ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1)         ||
        (rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2)
       )
    {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta) {
        ISP_MGR_G2C_T::getInstance(m_eSensorDev).setEnable(MTRUE);
        //}

       // ISP_MGR_SEEE_T::getInstance(m_eSensorDev).setEnable(MTRUE);

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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC_2(const ISP_INFO_T& rIspInfo)
{
	MBOOL bDisable = propertyGet("vendor.isp.lsc2.disable", 0);
    MBOOL fgRet = MFALSE;
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


#if 1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NR3D
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR3D(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 1

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
#if 1

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
#else
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
#endif

		// Invoke callback for customers to modify.
		if	( isDynamicTuning() )
		{	//	Dynamic Tuning: Enable
			m_pIspTuningCustom->refine_MIXER3(rIspCamInfo, m_IspNvramMgr, mixer3);
		}

		// Load it to ISP manager buffer
		putIspHWBuf(m_eSensorDev, mixer3 );

    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LCE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LCE(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 1   //this chip doesn't have LCE
    // Get default NVRAM parameter
	MUINT16 u2Idx = rIdx.getIdx_LCE();
	ISP_NVRAM_LCE_T lce = m_IspNvramMgr.getLCE(u2Idx);


    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_LCE(rIspCamInfo, m_IspNvramMgr, lce);
    }

    // Load it to ISP manager buffer
    putIspHWBuf(m_eSensorDev, lce);

#endif

    return  MTRUE;
}
#endif
