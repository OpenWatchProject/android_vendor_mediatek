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
#define LOG_TAG "paramctrl_validate"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_error_code.h>
//#include <mtkcam/featureio/aaa_hal_if.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include "paramctrl.h"
//#include <hwutils/CameraProfile.h>

#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <EModule_string.h>
#include <EApp_string.h>
#include <EIspProfile_string.h>
#include <ESensorMode_string.h>
#endif

using namespace android;
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validate(RequestSet_T const RequestSet, MBOOL const fgForce, MINT32 i4SubsampleIdex)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    NS3Av3::RequestSet_T rRequestSet = RequestSet;
    MINT32 i4FrameID = rRequestSet.vNumberSet.front();

    CAM_LOGD_IF(m_bDebugEnable,"%s(): i4FrameID = %d\n", __FUNCTION__, i4FrameID);

    // flush turning setting
    //TuningMgr::getInstance().flushSetting(m_eSoftwareScenario);


    MBOOL const fgRet = ( MERR_OK == (err = validateFrameless()) )
                    &&  ( MERR_OK == (err = validatePerFrame(rRequestSet, fgForce, i4SubsampleIdex)) )
                        ;
#if 0
    // apply default setting to for 1 ~ 6,
    // 1, 2, 3 are dummy buffers, will be P1 and AAO output
    // 4, 5, 6 are protection buffers
    for (MINT32 i = 1; i <= 6; i++)
    {
        //applyToHw_PerFrame_P1(i);
        validatePerFrame(i, fgForce);
    }
#endif
    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validateFrameless()
{
    MERROR_ENUM err = MERR_UNKNOWN;

    CAM_LOGD("[+validateFrameless]");

    Mutex::Autolock lock(m_Lock);

    //  (1)
    if  ( ! prepareHw_Frameless_All() )
    {
        err = MERR_PREPARE_HW;
        goto lbExit;
    }

    //  (2)
    if  ( ! applyToHw_Frameless_All() )
    {
        err = MERR_APPLY_TO_HW;
        goto lbExit;
    }

    //  (3) Force validatePerFrame() to run.
    m_u4ParamChangeCount++;

    err = MERR_OK;

lbExit:
#if ENABLE_MY_ERR
    if  ( MERR_OK != err )
    {
        CAM_LOGE("[-validateFrameless]err(%X)", err);
    }
#endif

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validatePerFrame(RequestSet_T const RequestSet, MBOOL const fgForce, MINT32 i4SubsampleIdex)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    MBOOL fgKeep = RequestSet.fgKeep;
    MINT32 i4FrameID = RequestSet.vNumberSet[0];
    CAM_LOGD_IF(m_bDebugEnable, "[validatePerFrame] i4FrameID(%d), keep(%d)", i4FrameID, fgKeep);

    Mutex::Autolock lock(m_Lock);

    m_DualPD_PureRaw = RequestSet.fgDisableP1;

    m_rIspCamInfo.u4Id = i4FrameID;
    m_rIspCamInfo.fgNeedKeepP1 = fgKeep;

    CAM_IDX_QRY_COMB rMapping_Info;
    IdxMgr::createInstance(static_cast<ESensorDev_T>(m_eSensorDev))->getMappingInfo(static_cast<ESensorDev_T>(m_eSensorDev), rMapping_Info, m_rIspCamInfo.u4Id);

    m_rIspCamInfo.rMapping_Info.eDriverIC = rMapping_Info.eDriverIC;
    m_rIspCamInfo.rMapping_Info.eApp = rMapping_Info.eApp;
    CAM_LOGD_IF(m_bDebugEnable,"[validatePerFrame] eDriverIC(%d)",m_Mapping_InfoP1.eDriverIC);

    MINT32 debugCache = property_get_int32("vendor.debug.p1.cache.mode", 0);
    if (memcmp(&(m_rIspCamInfo.rMapping_Info), &m_Mapping_InfoP1, sizeof(CAM_IDX_QRY_COMB)) || debugCache || (m_rIspCamInfo.eControlMode == MTK_CONTROL_MODE_OFF)){
        memcpy(&m_Mapping_InfoP1, &(m_rIspCamInfo.rMapping_Info), sizeof(CAM_IDX_QRY_COMB));
        m_bMappingQueryFlagP1 = MTRUE;
    } else {
        CAM_LOGD_IF(m_bDebugEnable,"[validatePerFrame] use last mapping info(%d)", m_rIspCamInfo.u4Id);
        m_bMappingQueryFlagP1 = MFALSE;
    }

#if 0
    if(getOperMode() == EOperMode_Meta) {
        err = MERR_OK;
        CAM_LOGD("[validatePerFrame] Meta Mode\n");
        goto lbExit;
    }
#endif

    if (m_bDebugEnable) {
        CAM_LOGD("%s()\n", __FUNCTION__);
        //IspDebug::getInstance().dumpIspDebugMessage();
    }

    //  (0) Make sure it's really needed to apply.
    if  ( 0 == getParamChangeCount()  //  no params change
          && !fgForce)                //  not force to apply
    {
        err = MERR_OK;
        //TuningMgr::getInstance().byPassSetting(m_eSoftwareScenario, i4FrameID);
        goto lbExit;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[validatePerFrame](ParamChangeCount, fgForce)=(%d, %d)", getParamChangeCount(), fgForce);

    //  (1) reset to 0 since all params have been applied.
    resetParamChangeCount();

    //  (2) Do something.
    if (fgKeep)
    {
        err = MERR_OK;
        prepareHw_PerFrame_LSC();
        if (!applyToHw_PerFrame_P1(RequestSet, i4SubsampleIdex))
        {
            err = MERR_SET_ISP_REG;
        }
    }
    else
    {
        err = do_validatePerFrame(RequestSet, i4SubsampleIdex);
    }
    if (MERR_OK != err)
    {
        CAM_LOGE("[validatePerFrame]do_validatePerFrame returns err(%d)", err);
        goto lbExit;
    }

    err = MERR_OK;

lbExit:

    return  err;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& rIspInfo, void* pTuningBuf)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    CAM_LOGD_IF(m_bDebugEnable, "[validatePerFrameP2] i4FrameID = %d", rIspInfo.rCamInfo.u4Id);

    //Mutex::Autolock lock(m_Lock);

#if 0
    if(getOperMode() == EOperMode_Meta) {
        err = MERR_OK;
        CAM_LOGD("[validatePerFrame] Meta Mode\n");
        goto lbExit;
    }
#endif

    void* pRegBuf = ((TuningParam*)pTuningBuf)->pRegBuf;
    void* pMfbBuf = ((TuningParam*)pTuningBuf)->pMfbBuf;
    void* pLcsBuf = ((TuningParam*)pTuningBuf)->pLcsBuf;

    MBOOL bSlave = ((TuningParam*)pTuningBuf)->bSlave;
    void* pDualSynInfo = ((TuningParam*)pTuningBuf)->pDualSynInfo;

    //  (2) Do something.

    setLCSOBuffer(pLcsBuf);

    if(pDualSynInfo){
        //slave get info
        if(bSlave) setDualSyncInfo(pDualSynInfo);
        else clearDualSyncInfo();
    }

    err = (do_validatePerFrameP2(flowType, rIspInfo, pRegBuf, pMfbBuf));

    if(pDualSynInfo){
        //master set info
        if(!bSlave) getDualSyncInfo(pDualSynInfo);
        else clearDualSyncInfo();
    }

    if  (MERR_OK != err)
    {
        CAM_LOGE("[validatePerFrameP2]do_validatePerFrame returns err(%d)", err);
        goto lbExit;
    }

    err = MERR_OK;

lbExit:

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
check_ISO_env_info()
{
    int temp_count = 0;

    if( m_rIspParam.ISPRegs.Iso_Env.u2Length > NVRAM_ISP_REGS_ISO_SUPPORT_NUM){
        CAM_LOGE("CUSTOM_ISO_ENV_LENGTH_ERROR");
        return MFALSE;
    }
    else{
        for(int i=0; i<(m_rIspParam.ISPRegs.Iso_Env.u2Length-1); i++)
        {
            if( (m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[i] < m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[i+1])&&
                (m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[i] > 0))
            {
                temp_count++;
            }
        }
        if ( temp_count != (m_rIspParam.ISPRegs.Iso_Env.u2Length-1) ){
            CAM_LOGE("CUSTOM_ISO_ENV_SETTING_ERROR");
            return MFALSE;
        }
    }
    return MTRUE;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
check_CT_env_info()
{
    int temp_count = 0;
    #if MTK_CAM_NEW_NVRAM_SUPPORT
    MINT32 COLOR_CT_Env[5] ={
        2000, 3500, 5000, 6500, 8000
    };
    for(int i=0; i<(ISP_NVRAM_COLOR_CT_NUM-1); i++)
    {
        if( (COLOR_CT_Env[i] < COLOR_CT_Env[i+1])&& (COLOR_CT_Env[i] > 0))
        {
            temp_count++;
        }
    }
    #else
    for(int i=0; i<(ISP_NVRAM_COLOR_CT_NUM-1); i++)
    {
        if( (m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[i] < m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[i+1])&&
            (m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[i] > 0))
        {
            temp_count++;
        }
    }
	#endif
    if ( temp_count != (ISP_NVRAM_COLOR_CT_NUM-1) ){
        CAM_LOGE("CUSTOM_ISO_ENV_SETTING_ERROR");
        return MFALSE;
    }
    return MTRUE;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
Paramctrl::getDefaultObc(MVOID*& pISP_NVRAM_Reg)
{
    pISP_NVRAM_Reg = &m_rIspParam.ISPRegs;
}

#define IDXCACHE_LOG(bQuery, module, index, info) \
    CAM_LOGD_IF(m_i4IdxCacheLogEnable, \
        "[IdxCache][From:%s][Dev:%d-Mod:%s(%d)] (Idx %d) (PF %s, SM %s, Bin %d, P2 %d, FLASH %d, APP %s, FD %d, ZOOM %d, LV %d, CT %d, ISO %d, IC %d)",\
        bQuery ? "IdxMgr":"Cache", m_eSensorDev, strEModule[module], module, index,  strEIspProfile[info.eIspProfile], strESensorMode[info.eSensorMode], info.eFrontBin, info.eP2size, info.eFlash, strEApp[info.eApp], info.eFaceDetection, info.eZoom_Idx, info.eLV_Idx, info.eCT_Idx, info.eISO_Idx, info.eDriverIC);

MUINT16
Paramctrl::
getISPIndex(MUINT32 u4IspInt, const RAWIspCamInfo& rIspCamInfo, MUINT32 module, MUINT16& index, MBOOL bQuery)
{
    CAM_IDX_QRY_COMB qry = rIspCamInfo.rMapping_Info;

    switch(u4IspInt)
    {
        case eIDX_UpperIso_UpperZoom:
        {
            qry.eISO_Idx =           (EISO_T)rIspCamInfo.eIdx_ISO_U;
            qry.eZoom_Idx =          (EZoom_T)rIspCamInfo.eIdx_Zoom_U;
            break;
        }
        case eIDX_LowerIso_UpperZoom:
        {
            qry.eISO_Idx =           (EISO_T)rIspCamInfo.eIdx_ISO_L;
            qry.eZoom_Idx =          (EZoom_T)rIspCamInfo.eIdx_Zoom_U;
            break;
        }
        case eIDX_UpperIso_LowerZoom:
        {
            qry.eISO_Idx =           (EISO_T)rIspCamInfo.eIdx_ISO_U;
            qry.eZoom_Idx =          (EZoom_T)rIspCamInfo.eIdx_Zoom_L;
            break;
        }
        case eIDX_LowerIso_LowerZoom:
        {
            qry.eISO_Idx =           (EISO_T)rIspCamInfo.eIdx_ISO_L;
            qry.eZoom_Idx =          (EZoom_T)rIspCamInfo.eIdx_Zoom_L;
            break;
        }
        case eIDX_LowerCT:
        {
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_L;
            break;
        }
        case eIDX_UpperCT:
        {
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_U;
            break;

        }
        case eIDX_LowerLV:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_L;
            break;
        }
        case eIDX_UpperLV:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_U;
            break;
        }
        case eIDX_UpperIso:
        {
            qry.eISO_Idx =           (EISO_T)rIspCamInfo.eIdx_ISO_U;
            break;
        }
        case eIDX_LowerIso:
        {
            qry.eISO_Idx =           (EISO_T)rIspCamInfo.eIdx_ISO_L;
            break;
        }
        case eIDX_LowerLv_LowerCt:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_L;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_L;
            break;
        }
        case eIDX_LowerLv_UpperCt:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_L;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_U;
            break;
        }
        case eIDX_UpperLv_LowerCt:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_U;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_L;
            break;
        }
        case eIDX_UpperLv_UpperCt:
        {
            qry.eLV_Idx =           (ELV_T)rIspCamInfo.eIdx_LV_U;
            qry.eCT_Idx =           (ECT_T)rIspCamInfo.eIdx_CT_U;
            break;
        }
        default:
        {
            qry.eISO_Idx =           (EISO_T)rIspCamInfo.rMapping_Info.eISO_Idx;
            qry.eZoom_Idx =          (EZoom_T)rIspCamInfo.rMapping_Info.eZoom_Idx;
            break;
        }
    }

    if (bQuery)
    {
        MUINT16 singleVal = 0;
        singleVal = m_pIdxMgr->query(m_eSensorDev, (EModule_T)module, qry, __FUNCTION__);
        index = singleVal;
    }

    IDXCACHE_LOG(bQuery, module, index, qry);

    return index;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
do_validatePerFrame(RequestSet_T const RequestSet ,MINT32 i4SubsampleIdex)
{
    MERROR_ENUM err = MERR_OK;
    MBOOL prepare_rdy;
    MINT32 time_ms;

    CAM_LOGD_IF(m_bDebugEnable, "[do_validatePerFrame]");

    if (m_pVCoreThrottling!=NULL && *m_pVCoreThrottling) {
        CAM_LOGW("VCore Thermal Throttling!!! Disable some ISP H/W");
    }

    //zHDR ISPProfile remapping
#if 0
    switch(m_rIspCamInfo.eIspProfile){
        case EIspProfile_zHDR_Preview:
            if (m_rIspCamInfo.eSensorMode == ESensorMode_Capture)
            {
                m_rIspCamInfo.eIspProfile = EIspProfile_zHDR_Capture;
            }
            break;
        case EIspProfile_zHDR_Video:
            if (m_rIspCamInfo.eSensorMode == ESensorMode_Preview || m_bFrontBinEn)
            {
                m_rIspCamInfo.eIspProfile = EIspProfile_zHDR_Preview;
            }
            break;
        case EIspProfile_zHDR_Capture:
            if (m_rIspCamInfo.eSensorMode == ESensorMode_Preview)
            {
                m_rIspCamInfo.eIspProfile = EIspProfile_zHDR_Preview;
            }
            break;
        default:
            break;
    }
#endif


    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.interp.ctrl", value, "1"); //default: enable
    m_IspInterpCtrl = atoi(value);

    //CPTLog(Event_Pipe_3A_ISP_VALIDATE_PERFRAME_DYNAMIC_TUNING, CPTFlagStart); // Profiling Start.
    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrame: nvram index", m_eSensorDev, m_bProfileEnable);

    //  (1) dynamic tuning
    if (isDynamicTuning())
    {
        // Refine camera info.
        m_pIspTuningCustom->refine_CamInfo(m_rIspCamInfo);

        m_rIspCamInfo.fgHDR = m_pIspTuningCustom->is_HDRTuning(m_rIspCamInfo);
    }

    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrame: prepareHw_PerFrame_All()", m_eSensorDev, m_bProfileEnable);
    //  (2) Apply Per-Frame Parameters.
    (isDynamicBypass() == MTRUE) ? (prepare_rdy = prepareHw_PerFrame_Partial())
                                 : (prepare_rdy = prepareHw_PerFrame_All());
    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    if (m_bProfileEnable)
        m_pAaaTimer->start("do_validatePerFrame: applyToHw_PerFrame_P1()", m_eSensorDev, m_bProfileEnable);

    if(!prepare_rdy || ! applyToHw_PerFrame_P1(RequestSet,i4SubsampleIdex))  //  Apply the ispmgr's buffer to H/W.
    {
        err = MERR_SET_ISP_REG;
        goto lbExit;
    }

#if 0
    if  (
            ! prepareHw_PerFrame_All()          //  Prepare param members to the ispmgr's buffer.
        ||  ! applyToHw_PerFrame_P1()          //  Apply the ispmgr's buffer to H/W.
        )
    {
        err = MERR_SET_ISP_REG;
        goto lbExit;
    }
#endif

lbExit:
    if  ( MERR_OK != err )
    {
        CAM_LOGE("[-do_validatePerFrame]err(%X)", err);
    }

    if (m_bProfileEnable)
        time_ms = m_pAaaTimer->End();

    return  err;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Paramctrl::getLCSparam(ISP_LCS_IN_INFO_T &lcs_info)
{
    ISP_MGR_LCS_T::getInstance(m_eSensorDev).get(lcs_info.lcs);
    lcs_info.fgOnOff = ISP_MGR_LCS_T::getInstance(m_eSensorDev).isEnable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID*
Paramctrl::getDMGItable(){

    if(ISP_MGR_BNR2_T::getInstance(m_eSensorDev).isPDC2Enable() &&
        ISP_MGR_BNR2_T::getInstance(m_eSensorDev).getDMGItable()){
        return ISP_MGR_BNR2_T::getInstance(m_eSensorDev).getDMGItable();
    }
    else if(ISP_MGR_BNR2_T::getInstance(m_eSensorDev).isBPC2Enable()){
        return ISP_MGR_BNR2_T::getInstance(m_eSensorDev).getDMGItable_Default();
    }
    else return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
do_validatePerFrameP2(MINT32 flowType, const ISP_INFO_T& _rIspInfo, void* pRegBuf, void* pMfbBuf)
{
    MERROR_ENUM err = MERR_OK;
    MBOOL prepare_rdy = MTRUE;

    CAM_LOGD_IF(m_bDebugEnable, "[do_validatePerFrameP2]");

    ISP_INFO_T rIspInfo_temp = _rIspInfo;

    //YUV Reprocess
    rIspInfo_temp.rCamInfo.rMapping_Info.eISO_Idx = map_ISO_value2index(rIspInfo_temp.rCamInfo.rAEInfo.u4RealISOValue);
    get_ISOIdx_neighbor(rIspInfo_temp.rCamInfo, rIspInfo_temp.rCamInfo.rMapping_Info.eISO_Idx, rIspInfo_temp.rCamInfo.rAEInfo.u4RealISOValue);
    rIspInfo_temp.rCamInfo.rMapping_Info.eCT_Idx = map_CT_value2index(rIspInfo_temp.rCamInfo.rAWBInfo.i4CCT);
    get_CTIdx_neighbor(rIspInfo_temp.rCamInfo, rIspInfo_temp.rCamInfo.rMapping_Info.eCT_Idx, rIspInfo_temp.rCamInfo.rAWBInfo.i4CCT);
    rIspInfo_temp.rCamInfo.rMapping_Info.eP2size = (EP2Size_T)m_pIspTuningCustom->map_P2inSize_to_MappingInfo(_rIspInfo.rCamInfo);
    rIspInfo_temp.rCamInfo.fgHDR = m_pIspTuningCustom->is_HDRTuning(rIspInfo_temp.rCamInfo);

    MINT32 debugCache = property_get_int32("vendor.debug.p1.cache.mode", 0);
    if (memcmp(&(rIspInfo_temp.rCamInfo.rMapping_Info), &m_Mapping_Info, sizeof(CAM_IDX_QRY_COMB)) || debugCache || (rIspInfo_temp.rCamInfo.eControlMode == MTK_CONTROL_MODE_OFF)){
        memcpy(&m_Mapping_Info, &(rIspInfo_temp.rCamInfo.rMapping_Info), sizeof(CAM_IDX_QRY_COMB));
        m_bMappingQueryFlag = MTRUE;
    } else {
        CAM_LOGD_IF(m_bDebugEnable,"[do_validatePerFrameP2] use last mapping info(%d)",rIspInfo_temp.rCamInfo.u4Id);
        m_bMappingQueryFlag = MFALSE;
    }
    const ISP_INFO_T &rIspInfo = rIspInfo_temp;

    //  (1) dynamic tuning
    if (isDynamicTuning())
    {
        const RAWIspCamInfo& rIspCamInfo = rIspInfo.rCamInfo;

        CAM_LOGD_IF(m_bDebugEnable, "[do_validatePerFrame()] isDynamicBypass() = %d\n", isDynamicBypass());
        (isDynamicBypass() == MTRUE) ? (prepare_rdy = prepareHw_PerFrame_Partial(rIspInfo))
                                     : (prepare_rdy = prepareHw_PerFrame_All(rIspInfo));
    }

    if(rIspInfo.rCamInfo.u1P2TuningUpdate != 2){

        if(pRegBuf){
            if( (!prepare_rdy) ||
                (! applyToHw_PerFrame_P2(flowType, rIspInfo, pRegBuf)))//  Apply the ispmgr's buffer to H/W.
            {
                err = MERR_SET_ISP_REG;
                goto lbExit;
            }
        }
    }
    else{
        CAM_LOGD("Fix tuning buf u1P2TuningUpdate=2, and skip apply tuning P2");

        //prepare BPCI table
        ISP_MGR_BNR2_T::getInstance(m_eSensorDev).apply(rIspInfo.rCamInfo, &m_RegNull);
    }

    if(pMfbBuf){
        prepareHw_PerFrame_MFB(rIspInfo.rCamInfo);
        applyToHw_PerFrame_MFB(flowType, rIspInfo, pMfbBuf);
    }

    IdxMgr::createInstance(static_cast<ESensorDev_T>(m_eSensorDev))->setMappingInfo(static_cast<ESensorDev_T>(m_eSensorDev), rIspInfo.rCamInfo.rMapping_Info, rIspInfo.rCamInfo.u4Id);

lbExit:
    if  ( MERR_OK != err )
    {
        CAM_LOGE("[-do_validatePerFrame]err(%X)", err);
    }

    return  err;
}

#if 0

MERROR_ENUM
ParamctrlComm::
setEnable_Meta_Gamma(MBOOL const fgForceEnable)
{
    CAM_LOGD(
        "[+setEnable_Meta_Gamma] (fgForceEnable, m_fgForceEnable_Meta_Gamma)=(%d, %d)"
        , fgForceEnable, m_fgForceEnable_Meta_Gamma
    );

    Mutex::Autolock lock(m_Lock);

    checkParamChange(m_fgForceEnable_Meta_Gamma, fgForceEnable);

    m_fgForceEnable_Meta_Gamma = fgForceEnable;

    return  MERR_OK;
}

#endif

