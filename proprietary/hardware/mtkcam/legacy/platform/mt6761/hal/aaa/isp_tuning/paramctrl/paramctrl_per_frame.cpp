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
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include <pca_mgr.h>
#include <mtkcam/algorithm/lib3a/dynamic_ccm.h>
#include <mtkcam/algorithm/lib3a/isp_interpolation.h>
#include <ccm_mgr.h>
#include <lsc_mgr2.h>
#include "paramctrl_if.h"
#include "paramctrl.h"
#include <mtkcam/hwutils/CameraProfile.h>
#include "mtkcam/featureio/vfb_hal_base.h"
//#include <mtkcam/featureio/capturenr.h>

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

    MY_LOG_IF(m_bDebugEnable,"[%s] i4FrameID(0x%8x)\n", __FUNCTION__, i4FrameID);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aaa.pvlog.enable", value, "0");
    int r3ALogEnable = atoi(value);
    property_get("vendor.isp.interp.ctrl", value, "1"); //default: enable
    m_IspInterpCtrl = atoi(value);

    AaaTimer localTimer("applyToHwAll", m_eSensorDev, (r3ALogEnable & EN_3A_SCHEDULE_LOG));

    TuningMgr::getInstance().dequeBuffer(m_eSoftwareScenario, i4FrameID);

    fgRet = ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_NSL2_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_PGN_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_UDM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_CCM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_GGM_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_G2C_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_G2C_SHADE_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_NBC_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_NBC2_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_SEEE_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        //&&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(m_rIspCamInfo.eIspProfile)
        ;

    TuningMgr::getInstance().enqueBuffer(m_eSoftwareScenario, i4FrameID);

    localTimer.End();

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
        fgRet = ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).apply(rIspCamInfo.eIspProfile, pReg);
        if (fgRet)
          MY_LOG("[%s] PureRaw Flow pass2 return TRUE", __FUNCTION__);
    }
    if (flowType == 0)
    {
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
            &&  ISP_MGR_NR3D_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_MFB_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
            &&  ISP_MGR_MIXER3_T::getInstance(m_eSensorDev).apply(rIspCamInfo.eIspProfile, pReg)
            ;
        MY_LOG("[%s] Processed Flow", __FUNCTION__);
    }
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
        &&  ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
        &&  ISP_MGR_OBC_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
        &&  ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).reset()
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
        MY_ERR("reset error");
        goto lbExit;
    }

    //  (3) prepare something and fill buffers.
    fgRet = MTRUE
        &&  prepareHw_PerFrame_DBS()
        &&  prepareHw_PerFrame_OBC()
        &&  prepareHw_PerFrame_BPC()
        &&  prepareHw_PerFrame_NR1()
        &&  prepareHw_PerFrame_LSC()
        &&  prepareHw_PerFrame_RPG()
        //&&  prepareHw_PerFrame_PGN()
        //&&  prepareHw_PerFrame_UDM()
        &&  prepareHw_PerFrame_CCM()
        &&  prepareHw_PerFrame_GGM()
        //&&  prepareHw_PerFrame_ANR()
        //&&  prepareHw_PerFrame_ANR2()
        //&&  prepareHw_PerFrame_CCR()
        //&&  prepareHw_PerFrame_BOK()
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
prepareHw_PerFrame_All(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] +", __FUNCTION__);

    MBOOL fgRet = MTRUE
        &&  prepareHw_PerFrame_PGN(rIspCamInfo, rIdx)
        &&  prepareHw_PerFrame_UDM(const_cast<RAWIspCamInfo&>(rIspCamInfo), rIdx)
        &&  prepareHw_PerFrame_CCM(rIspCamInfo, rIdx)
        &&  prepareHw_PerFrame_GGM(rIspCamInfo, rIdx)
        &&  prepareHw_PerFrame_ANR(const_cast<RAWIspCamInfo&>(rIspCamInfo), rIdx)
        &&  prepareHw_PerFrame_ANR2(const_cast<RAWIspCamInfo&>(rIspCamInfo), rIdx)
        &&  prepareHw_PerFrame_CCR(const_cast<RAWIspCamInfo&>(rIspCamInfo), rIdx)
        &&  prepareHw_PerFrame_BOK(const_cast<RAWIspCamInfo&>(rIspCamInfo), rIdx)
        &&  prepareHw_PerFrame_PCA(rIspCamInfo, rIdx)
        &&  prepareHw_PerFrame_EE(const_cast<RAWIspCamInfo&>(rIspCamInfo), rIdx)
        &&  prepareHw_PerFrame_EFFECT(rIspCamInfo, rIdx)
        &&  prepareHw_PerFrame_NR3D(rIspCamInfo, rIdx)
        &&  prepareHw_PerFrame_MFB(rIspCamInfo, rIdx)
        &&  prepareHw_PerFrame_MIXER3(rIspCamInfo, rIdx)
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
    return MTRUE;
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
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.dbs.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTEnable())){
        ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_DBS_T::getInstance(m_eSensorDev, m_eSensorTG).setEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_DBS_T dbs = m_IspNvramMgr.getDBS();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl)
        {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    MUINT32 u4DBSUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    MUINT32 u4DBSLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    MUINT16 u2DBSUpperIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO+1).DBS;
                    MUINT16 u2DBSLowerIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO).DBS;

                    //MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                    //          __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                    //          u4DBSUpperISO,
                    //          u4DBSLowerISO,
                    //          m_rIspCamInfo.eIdx_ISO+1,
                    //          m_rIspCamInfo.eIdx_ISO);

                    //MY_LOG_IF(m_bDebugEnable,"%s ISP_NVRAM_DBS_TH1_T: Upper(%d, %x), Lower(%d, %x)"
                    //    , __FUNCTION__
                    //    , u2DBSUpperIdx, m_IspNvramMgr.getDBS(u2DBSUpperIdx).th1.val
                    //    , u2DBSLowerIdx, m_IspNvramMgr.getDBS(u2DBSLowerIdx).th1.val);

                    SmoothDBS(m_rIspCamInfo.u4ISOValue,  // Real ISO
                             u4DBSUpperISO, // Upper ISO
                             u4DBSLowerISO, // Lower ISO
                             m_IspNvramMgr.getDBS(u2DBSUpperIdx), // DBS settings for upper ISO
                             m_IspNvramMgr.getDBS(u2DBSLowerIdx), // DBS settings for lower ISO
                             dbs);  // Output
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    MUINT32 u4DBSUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    MUINT32 u4DBSLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    MUINT16 u2DBSUpperIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO).DBS;
                    MUINT16 u2DBSLowerIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO-1).DBS;

                    //MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                    //          __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                    //          u4DBSUpperISO,
                    //          u4DBSLowerISO,
                    //          m_rIspCamInfo.eIdx_ISO,
                    //          m_rIspCamInfo.eIdx_ISO-1);

                    //MY_LOG_IF(m_bDebugEnable,"%s ISP_NVRAM_DBS_TH1_T: Upper(%d, %x), Lower(%d, %x)"
                    //    , __FUNCTION__
                    //    , u2DBSUpperIdx, m_IspNvramMgr.getDBS(u2DBSUpperIdx).th1.val
                    //    , u2DBSLowerIdx, m_IspNvramMgr.getDBS(u2DBSLowerIdx).th1.val);

                    SmoothDBS(m_rIspCamInfo.u4ISOValue,  // Real ISO
                             u4DBSUpperISO, // Upper ISO
                             u4DBSLowerISO, // Lower ISO
                             m_IspNvramMgr.getDBS(u2DBSUpperIdx), // DBS settings for upper ISO
                             m_IspNvramMgr.getDBS(u2DBSLowerIdx), // DBS settings for lower ISO
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
        putIspHWBuf(m_eSensorDev, m_eSensorTG, dbs );
    }
#endif
    return  MTRUE;

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_OBC()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.obc.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_DynamicBypass_OBC()
{
#if 1
    // Get backup NVRAM parameter
    ISP_NVRAM_OBC_T obc;

    this->getPureOBCInfo(&obc);

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, m_eSensorTG, obc );
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_BPC()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.bpc.disable", value, "1"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTBPCEnable())){
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setBPCEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_BPC_T bpc = m_IspNvramMgr.getBPC();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl) {
             if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
                 if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    MUINT32 u4BPCUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                    MUINT32 u4BPCLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    MUINT16 u2BPCUpperIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO+1).BPC;
                    MUINT16 u2BPCLowerIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO).BPC;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              u4BPCUpperISO,
                              u4BPCLowerISO,
                              m_rIspCamInfo.eIdx_ISO+1,
                              m_rIspCamInfo.eIdx_ISO);

                    MY_LOG_IF(m_bDebugEnable,"%s ISP_NVRAM_BNR_BPC_TH1_T: Upper(%d, %x), Lower(%d, %x)"
                        , __FUNCTION__
                        , u2BPCUpperIdx, m_IspNvramMgr.getBPC(u2BPCUpperIdx).th1.val
                        , u2BPCLowerIdx, m_IspNvramMgr.getBPC(u2BPCLowerIdx).th1.val);
#if 0
                    SmoothBPC(m_rIspCamInfo.u4ISOValue,  // Real ISO
                             u4BPCUpperISO, // Upper ISO
                             u4BPCLowerISO, // Lower ISO
                             m_IspNvramMgr.getBPC(u2BPCUpperIdx), // BPC settings for upper ISO
                             m_IspNvramMgr.getBPC(u2BPCLowerIdx), // BPC settings for lower ISO
                             bpc);  // Output
#endif
                 }
             }
             else {
                 if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                    MUINT32 u4BPCUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                    MUINT32 u4BPCLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                    MUINT16 u2BPCUpperIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO).BPC;
                    MUINT16 u2BPCLowerIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO-1).BPC;

                    MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                              __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                              u4BPCUpperISO,
                              u4BPCLowerISO,
                              m_rIspCamInfo.eIdx_ISO,
                              m_rIspCamInfo.eIdx_ISO-1);

                    MY_LOG_IF(m_bDebugEnable,"%s ISP_NVRAM_BNR_BPC_TH1_T: Upper(%d, %x), Lower(%d, %x)"
                        , __FUNCTION__
                        , u2BPCUpperIdx, m_IspNvramMgr.getBPC(u2BPCUpperIdx).th1.val
                        , u2BPCLowerIdx, m_IspNvramMgr.getBPC(u2BPCLowerIdx).th1.val);
#if 0
                    SmoothBPC(m_rIspCamInfo.u4ISOValue,  // Real ISO
                             u4BPCUpperISO, // Upper ISO
                             u4BPCLowerISO, // Lower ISO
                             m_IspNvramMgr.getBPC(u2BPCUpperIdx), // BPC settings for upper ISO
                             m_IspNvramMgr.getBPC(u2BPCLowerIdx), // BPC settings for lower ISO
                             bpc);  // Output
#endif
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NR1()
{
#if 1

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.nr1.disable", value, "1"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).isCCTCTEnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video) ||
        (m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
        ((m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)&&(isRPGEnable()))) {
        ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setCTEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_BNR_T::getInstance(m_eSensorDev, m_eSensorTG).setCTEnable(MTRUE);

        // Get default NVRAM parameter
        ISP_NVRAM_BNR_NR1_T nr1 = m_IspNvramMgr.getNR1();

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo) && m_IspInterpCtrl) {
           if (m_rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO)) {
              if (m_rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                 MUINT32 u4NR1UpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO+1));
                 MUINT32 u4NR1LowerISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                 MUINT16 u2NR1UpperIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO+1).NR1;
                 MUINT16 u2NR1LowerIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO).NR1;

                 MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                            u4NR1UpperISO,
                            u4NR1LowerISO,
                            m_rIspCamInfo.eIdx_ISO+1,
                            m_rIspCamInfo.eIdx_ISO);
                 MY_LOG_IF(m_bDebugEnable,"%s ISP_NVRAM_NR1_CT_CON_T: Upper(%d, %x), Lower(%d, %x)"
                           , __FUNCTION__
                           , u2NR1UpperIdx, m_IspNvramMgr.getNR1(u2NR1UpperIdx).ct_con.val
                           , u2NR1LowerIdx, m_IspNvramMgr.getNR1(u2NR1LowerIdx).ct_con.val);
#if 0
                 SmoothNR1(m_rIspCamInfo.u4ISOValue,  // Real ISO
                           u4NR1UpperISO, // Upper ISO
                           u4NR1LowerISO, // Lower ISO
                           m_IspNvramMgr.getNR1(u2NR1UpperIdx), // NR1 settings for upper ISO
                           m_IspNvramMgr.getNR1(u2NR1LowerIdx), // NR1 settings for lower ISO
                           nr1);  // Output
#endif
              }
           }
           else {
              if (m_rIspCamInfo.eIdx_ISO > eIDX_ISO_100) {
                 MUINT32 u4NR1UpperISO = m_pIspTuningCustom->map_ISO_index_to_value(m_rIspCamInfo.eIdx_ISO);
                 MUINT32 u4NR1LowerISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(m_rIspCamInfo.eIdx_ISO-1));
                 MUINT16 u2NR1UpperIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO).NR1;
                 MUINT16 u2NR1LowerIdx = getISPIndex_byISO(m_rIspCamInfo.eIdx_ISO-1).NR1;

                 MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperISOIdx = %d, u2LowerISOIdx = %d\n",
                            __FUNCTION__, m_rIspCamInfo.u4ISOValue,
                            u4NR1UpperISO,
                            u4NR1LowerISO,
                            m_rIspCamInfo.eIdx_ISO,
                            m_rIspCamInfo.eIdx_ISO-1);
                 MY_LOG_IF(m_bDebugEnable,"%s ISP_NVRAM_NR1_CT_CON_T: Upper(%d, %x), Lower(%d, %x)"
                           , __FUNCTION__
                           , u2NR1UpperIdx, m_IspNvramMgr.getNR1(u2NR1UpperIdx).ct_con.val
                           , u2NR1LowerIdx, m_IspNvramMgr.getNR1(u2NR1LowerIdx).ct_con.val);
#if 0
                 SmoothNR1(m_rIspCamInfo.u4ISOValue,  // Real ISO
                           u4NR1UpperISO, // Upper ISO
                           u4NR1LowerISO, // Lower ISO
                           m_IspNvramMgr.getNR1(u2NR1UpperIdx), // NR1 settings for upper ISO
                           m_IspNvramMgr.getNR1(u2NR1LowerIdx), // NR1 settings for lower ISO
                           nr1);  // Output
#endif
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_LSC()
{
#if 1
    MY_LOG_IF(m_bDebugEnable,"%s", __FUNCTION__);

    //////////////////////////////////////
    MUINT32 new_cct_idx = eIDX_Shading_CCT_BEGIN;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.lsc_mgr.ratio", value, "-1");
    MINT32 i4Rto = atoi(value);

    // Check to see if it is needed to load LUT.
    MY_LOG_IF(m_bDebugEnable,"[%s] m_pLscMgr OperMode(%d)\n", __FUNCTION__, getOperMode());

    // Invoke callback for customers to modify.
    if  (m_fgDynamicShading)
    {
        // Dynamic Tuning: Enable
        new_cct_idx = m_pIspTuningCustom->evaluate_Shading_CCT_index(m_rIspCamInfo);
        m_pLscMgr->setCTIdx(new_cct_idx);

        if (i4Rto == -1)
        {
            i4Rto = m_pIspTuningCustom->evaluate_Shading_Ratio(m_rIspCamInfo);
            MY_LOG("[%s] (flash, iso, rto) = (%d, %d, %d)", __FUNCTION__,
                m_rIspCamInfo.rFlashInfo.isFlash,
                m_rIspCamInfo.rAEInfo.u4RealISOValue, i4Rto);
        }
    }

    else
    {
        if (i4Rto == -1) i4Rto = 32;
    }

    m_pLscMgr->setLscRatio(i4Rto);

    if (!m_pLscMgr->getTsfOnOff())
        m_pLscMgr->updateLsc();

    // debug message
    m_rIspCamInfo.eIdx_Shading_CCT = (NSIspTuning::EIndex_Shading_CCT_T)m_pLscMgr->getCTIdx();
    m_IspNvramMgr.setIdx_LSC(m_pLscMgr->getRegIdx());
    //////////////////////////////////////
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NSL2()
{
#if 1
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
#endif
    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_RPG()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.rpg.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.pgn.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_UDM()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.udm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_UDM(m_rIspCamInfo, m_IspNvramMgr, udm);
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
        udm.dsb.bits.UDM_FL_MODE = 1;
    }
    else {
        udm.dsb.bits.UDM_FL_MODE = 0;     //cfa.byp.bit     DM_FG_MODE
    }


    if ((!ISP_MGR_UDM_T::getInstance(m_eSensorDev).isCCTEnable()) || (bDisable)) // CCT usage: fix CFA index
    {
        udm = m_IspNvramMgr.getUDM(NVRAM_UDM_DISABLE_IDX);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, udm );
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ggm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    ISP_NVRAM_GGM_T ggm;

    if ((bDisable) ||
        (!ISP_MGR_GGM_T::getInstance(m_eSensorDev).isCCTEnable())              ||
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
        ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_GGM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        if (m_rIspCamInfo.eToneMapMode == MTK_TONEMAP_MODE_CONTRAST_CURVE)
        {
            m_rIspCamInfo.rMtkGGM = m_pGgmMgr->getGGM();
        }
        else
        {
            // Get default NVRAM parameter
            if ((m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
                (m_rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
                (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
                (m_rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video)) {
                ggm = m_IspNvramMgr.getIHDRGGM(m_rIspCamInfo.rAEInfo.i4GammaIdx);
                MY_LOG_IF(m_bDebugEnable,"%s: m_rIspCamInfo.rAEInfo.i4GammaIdx = %d", __FUNCTION__, m_rIspCamInfo.rAEInfo.i4GammaIdx);
            }
            else {
                ggm = m_IspNvramMgr.getGGM();
            }

            // Invoke callback for customers to modify.
            if ( isDynamicTuning() )
            {   //  Dynamic Tuning: Enable
                m_pIspTuningCustom->refine_GGM(m_rIspCamInfo,  m_IspNvramMgr, ggm);
                m_pIspTuningCustom->userSetting_EFFECT_GGM(m_rIspCamInfo, m_eIdx_Effect, ggm);
            }

            // Load it to ISP manager buffer.
            //putIspHWBuf(m_eSensorDev, ggm );
            m_rIspCamInfo.rMtkGGM = ggm;
            m_pGgmMgr->updateGGM(ggm);
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
prepareHw_PerFrame_ANR()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.anr.disable", value, "0"); // 0: enable, 1: disable
    //0807forNE, disable ANR module
    MBOOL bDisable = atoi(value);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(m_rIspCamInfo);

    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANR1Enable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)&&(isRPGEnable())) ) {

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
                    m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx = m_IspNvramMgr.getIdx_ANR()-1;

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
#endif
    return  MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR2()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.anr2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(m_rIspCamInfo);

    #if 1
    if ((bDisable) ||
         (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTANR2Enable()) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off)      ||
         (m_rIspCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR)||
        (m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF)     ||
        ((m_rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)&&(isRPGEnable())))
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ccr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTCCREnable()) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)       ||
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_BOK()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.bok.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTCCREnable())){
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


MBOOL
Paramctrl::
prepareHw_PerFrame_PCA()
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.pca.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
    }


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
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ee.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
        (m_rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)||
        (m_rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_OFF)                   ||
        ((m_rIspCamInfo.eEdgeMode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG)&&(isRPGEnable()))) {
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

                    SmoothEE(m_rIspCamInfo.u4ISOValue,  // Real ISO
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

                    SmoothEE(m_rIspCamInfo.u4ISOValue,  // Real ISO
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EFFECT()
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.g2c.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_NSL2(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
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


#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_PGN(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.pgn.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_UDM(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.udm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);
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
    MUINT16 u2Idx = rIdx.getIdx_UDM();
    ISP_NVRAM_UDM_T udm = m_IspNvramMgr.getUDM(u2Idx);

    if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
        if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
             if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                rIspCamInfo.rIspIntInfo.u4UdmUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                rIspCamInfo.rIspIntInfo.u4UdmLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                rIspCamInfo.rIspIntInfo.u2UdmUpperIdx = m_IspNvramMgr.getIdx_UDM()+1;
                rIspCamInfo.rIspIntInfo.u2UdmLowerIdx = m_IspNvramMgr.getIdx_UDM();

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, rIspCamInfo.u4ISOValue,
                          rIspCamInfo.rIspIntInfo.u4UdmUpperISO,
                          rIspCamInfo.rIspIntInfo.u4UdmLowerISO,
                          rIspCamInfo.rIspIntInfo.u2UdmUpperIdx,
                          rIspCamInfo.rIspIntInfo.u2UdmLowerIdx);

                SmoothUDM(rIspCamInfo.u4ISOValue,  // Real ISO
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
                rIspCamInfo.rIspIntInfo.u2UdmUpperIdx = m_IspNvramMgr.getIdx_UDM();
                rIspCamInfo.rIspIntInfo.u2UdmLowerIdx = m_IspNvramMgr.getIdx_UDM()-1;

                MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                          __FUNCTION__, rIspCamInfo.u4ISOValue,
                          rIspCamInfo.rIspIntInfo.u4UdmUpperISO,
                          rIspCamInfo.rIspIntInfo.u4UdmLowerISO,
                          rIspCamInfo.rIspIntInfo.u2UdmUpperIdx,
                          rIspCamInfo.rIspIntInfo.u2UdmLowerIdx);

                SmoothUDM(rIspCamInfo.u4ISOValue,  // Real ISO
                rIspCamInfo.rIspIntInfo.u4UdmUpperISO, // Upper ISO
                rIspCamInfo.rIspIntInfo.u4UdmLowerISO, // Lower ISO
                m_IspNvramMgr.getUDM(rIspCamInfo.rIspIntInfo.u2UdmUpperIdx), // CFA settings for upper ISO
                m_IspNvramMgr.getUDM(rIspCamInfo.rIspIntInfo.u2UdmLowerIdx), // CFA settings for lower ISO
                udm);  // Output
             }
         }
    }

    // Invoke callback for customers to modify.
    if  ( isDynamicTuning() )
    {   //  Dynamic Tuning: Enable
        m_pIspTuningCustom->refine_UDM(rIspCamInfo, m_IspNvramMgr, udm);
    }

    // FG mode protection
    if (fgRPGEnable && ((rIspCamInfo.eIspProfile == EIspProfile_Preview)      ||
                          (rIspCamInfo.eIspProfile == EIspProfile_Video)        ||
                          (rIspCamInfo.eIspProfile == EIspProfile_N3D_Preview)  ||
                          (rIspCamInfo.eIspProfile == EIspProfile_N3D_Video)    ||
                          (rIspCamInfo.eIspProfile == EIspProfile_IHDR_Preview) ||
                          (rIspCamInfo.eIspProfile == EIspProfile_IHDR_Video)   ||
                          (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Preview) ||
                          (rIspCamInfo.eIspProfile == EIspProfile_MHDR_Video))) {
        udm.dsb.bits.UDM_FL_MODE = 1;
    }
    else {
        udm.dsb.bits.UDM_FL_MODE = 0;
    }


    if ((!ISP_MGR_UDM_T::getInstance(m_eSensorDev).isCCTEnable()) || (bDisable)) // CCT usage: fix CFA index
    {
        udm = m_IspNvramMgr.getUDM(NVRAM_UDM_DISABLE_IDX);
    }

    // Load it to ISP manager buffer.
    putIspHWBuf(m_eSensorDev, udm );

#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.ccm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_MultiPass_ANR_2)) {
        ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_CCM_T::getInstance(m_eSensorDev).setEnable(MTRUE);

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, rIspCamInfo.rMtkCCM);
    }
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_GGM(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ggm.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.anr.disable", value, "0"); // 0: enable, 1: disable
    //0807forNE, disable ANR module
    MBOOL bDisable = atoi(value);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(rIspCamInfo);

    if ((bDisable) ||
        (!ISP_MGR_NBC_T::getInstance(m_eSensorDev).isCCTANR1Enable()) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)) {

        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR1Enable(MFALSE);
    }
    else if ((rIspCamInfo.eIspProfile == EIspProfile_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_PureRAW_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {


        ISP_MGR_NBC_T::getInstance(m_eSensorDev).setANR1_ENYEnable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_ANR();
        ISP_NVRAM_ANR_T anr = m_IspNvramMgr.getANR(u2Idx);

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
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

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
        }

        // set ANR_LCE_LINK
#warning "SWNR linker error"
#if 0
        SwNRParam::getInstance(m_i4SensorIdx)->setANR_LCE_LINK(static_cast<MBOOL>(anr.con1.bits.ANR_LCE_LINK));
        m_pLscMgr->setSwNr();
#endif
        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
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

        // Invoke callback for customers to modify.
        if  ( isDynamicTuning() )
        {   //  Dynamic Tuning: Enable
            m_pIspTuningCustom->refine_ANR(rIspCamInfo, m_IspNvramMgr, anr);
        }

        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr );
    }
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_ANR2(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.anr2.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    m_u4SwnrEncEnableIsoThreshold = m_pIspTuningCustom->get_SWNR_ENC_enable_ISO_threshold(rIspCamInfo);

    if ((bDisable) ||
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTANR2Enable()) ||
        (rIspCamInfo.eNRMode == MTK_NOISE_REDUCTION_MODE_OFF) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR)||
        (m_rIspCamInfo.eIspProfile == EIspProfile_VFB_PostProc)) {

        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2Enable(MFALSE);
    }
    else if ((rIspCamInfo.eIspProfile == EIspProfile_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_VSS_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_PureRAW_Capture_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR) ||
             (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {

        if (rIspCamInfo.u4ISOValue >= m_u4SwnrEncEnableIsoThreshold) {
            //ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2_ENCEnable(MTRUE);
            ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2_ENYEnable(MTRUE);
        }
        else {
            //ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2_ENCEnable(MFALSE);
            ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setANR2_ENYEnable(MFALSE);
        }

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_ANR2();
        ISP_NVRAM_ANR2_T anr2 = m_IspNvramMgr.getANR2(u2Idx);

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
             if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                 if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                    rIspCamInfo.rIspIntInfo.u4AnrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                    rIspCamInfo.rIspIntInfo.u4AnrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                    rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = m_IspNvramMgr.getIdx_ANR2()+1;
                    rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = m_IspNvramMgr.getIdx_ANR2();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(rIspCamInfo.u4ISOValue,  // Real ISO
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
                    rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = m_IspNvramMgr.getIdx_ANR2();
                    rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = m_IspNvramMgr.getIdx_ANR2()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(rIspCamInfo.u4ISOValue,  // Real ISO
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

        // set ANR_LCE_LINK
#warning "SWNR linker error"
#if 0
        SwNRParam::getInstance(m_i4SensorIdx)->setANR_LCE_LINK(static_cast<MBOOL>(anr.con1.bits.ANR_LCE_LINK));
        m_pLscMgr->setSwNr();
#endif
        // Load it to ISP manager buffer.
        putIspHWBuf(m_eSensorDev, anr2 );
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
                    rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = m_IspNvramMgr.getIdx_ANR2()+1;
                    rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = m_IspNvramMgr.getIdx_ANR2();

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(rIspCamInfo.u4ISOValue,  // Real ISO
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
                    rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx = m_IspNvramMgr.getIdx_ANR2();
                    rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx = m_IspNvramMgr.getIdx_ANR2()-1;

                    MY_LOG_IF(m_bDebugEnable,"%s(): rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                              __FUNCTION__, rIspCamInfo.u4ISOValue,
                              rIspCamInfo.rIspIntInfo.u4AnrUpperISO,
                              rIspCamInfo.rIspIntInfo.u4AnrLowerISO,
                              rIspCamInfo.rIspIntInfo.u2Anr2UpperIdx,
                              rIspCamInfo.rIspIntInfo.u2Anr2LowerIdx);

                    SmoothANR2(rIspCamInfo.u4ISOValue,  // Real ISO
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
#endif

    return  MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_CCR(RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ccr.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if ((bDisable) ||
        (!ISP_MGR_NBC2_T::getInstance(m_eSensorDev).isCCTCCREnable()) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off)      ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR) ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing)       ||
        (rIspCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR)) {
        ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setCCREnable(MFALSE);
    }
    else {
        //if (getOperMode() != EOperMode_Meta)
            ISP_MGR_NBC2_T::getInstance(m_eSensorDev).setCCREnable(MTRUE);

        // Get default NVRAM parameter
        MUINT16 u2Idx = rIdx.getIdx_CCR();
        ISP_NVRAM_CCR_T ccr = m_IspNvramMgr.getCCR(u2Idx);

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl)

        {
                if (rIspCamInfo.u4ISOValue >= m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO)) {
                     if (rIspCamInfo.eIdx_ISO < eIDX_ISO_3200) {
                        rIspCamInfo.rIspIntInfo.u4CcrUpperISO = m_pIspTuningCustom->map_ISO_index_to_value(static_cast<EIndex_ISO_T>(rIspCamInfo.eIdx_ISO+1));
                        rIspCamInfo.rIspIntInfo.u4CcrLowerISO = m_pIspTuningCustom->map_ISO_index_to_value(rIspCamInfo.eIdx_ISO);
                        rIspCamInfo.rIspIntInfo.u2CcrUpperIdx = m_IspNvramMgr.getIdx_CCR()+1;
                        rIspCamInfo.rIspIntInfo.u2CcrLowerIdx = m_IspNvramMgr.getIdx_CCR();

                        MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                                  __FUNCTION__, rIspCamInfo.u4ISOValue,
                                  rIspCamInfo.rIspIntInfo.u4CcrUpperISO,
                                  rIspCamInfo.rIspIntInfo.u4CcrLowerISO,
                                  rIspCamInfo.rIspIntInfo.u2CcrUpperIdx,
                                  rIspCamInfo.rIspIntInfo.u2CcrLowerIdx);

                        SmoothCCR(rIspCamInfo.u4ISOValue,  // Real ISO
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
                        rIspCamInfo.rIspIntInfo.u2CcrUpperIdx = m_IspNvramMgr.getIdx_CCR();
                        rIspCamInfo.rIspIntInfo.u2CcrLowerIdx = m_IspNvramMgr.getIdx_CCR()-1;

                        MY_LOG_IF(m_bDebugEnable,"%s(): m_rIspCamInfo.u4ISOValue = %d, u4UpperISO = %d, u4LowerISO = %d, u2UpperIdx = %d, u2LowerIdx = %d\n",
                                  __FUNCTION__, rIspCamInfo.u4ISOValue,
                                  rIspCamInfo.rIspIntInfo.u4CcrUpperISO,
                                  rIspCamInfo.rIspIntInfo.u4CcrLowerISO,
                                  rIspCamInfo.rIspIntInfo.u2CcrUpperIdx,
                                  rIspCamInfo.rIspIntInfo.u2CcrLowerIdx);

                        SmoothCCR(rIspCamInfo.u4ISOValue,  // Real ISO
                        rIspCamInfo.rIspIntInfo.u4CcrUpperISO, // Upper ISO
                        rIspCamInfo.rIspIntInfo.u4CcrLowerISO, // Lower ISO
                        m_IspNvramMgr.getCCR(rIspCamInfo.rIspIntInfo.u2CcrUpperIdx), // CFA settings for upper ISO
                        m_IspNvramMgr.getCCR(rIspCamInfo.rIspIntInfo.u2CcrLowerIdx), // CFA settings for lower ISO
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


MBOOL
Paramctrl::
prepareHw_PerFrame_BOK(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)

{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.bok.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

MBOOL
Paramctrl::
prepareHw_PerFrame_PCA(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.pca.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    MBOOL fgIsToLoadLut = MFALSE;   //  MTRUE indicates to load LUT.

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
    }


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
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.ee.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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

        if (m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo) && m_IspInterpCtrl) {
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
#endif
    return  MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
prepareHw_PerFrame_EFFECT(const RAWIspCamInfo& rIspCamInfo, const IndexMgr& rIdx)
{
#if 0
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("isp.g2c.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

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
#endif
    return  MTRUE;
}

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
