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
#define LOG_TAG "paramctrl_attributes"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <sys/stat.h>
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
//#include <awb_param.h>
//#include <ae_param.h>
//#include <af_param.h>
//#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <ispfeature.h>
#include <ccm_mgr.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <dce_mgr.h>
#include <lsc/ILscMgr.h>
#include <isp_mgr.h>
#include <isp_mgr_config.h>
#include <mtkcam/drv/IHalSensor.h>
#include "paramctrl.h"
#include <isp_tuning_mgr.h>
//#include <librwb/MTKRWB.h>
#include <nvbuf_util.h>
#include <nvram_drv.h>

#include <cutils/properties.h>
#include <isp_interpolation/isp_interpolation.h>

#include <ATMsEventMgr.h>
#include <string>

using namespace NSIspTuning;


#define _A_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

#define _A_P2(reg)\
    ((MUINT32)offsetof(dip_x_reg_t, reg))

MVOID
Paramctrl::
enableDynamicTuning(MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[+enableDynamicTuning](old, new)=(%d, %d)", m_fgDynamicTuning, fgEnable);

    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicTuning, fgEnable) )
    {
        m_fgDynamicTuning = fgEnable;
    }
}


MBOOL
Paramctrl::
enableDynamicCCM(MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[+enableDynamicCCM](old, new)=(%d, %d)", m_fgDynamicCCM, fgEnable);

    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicCCM, fgEnable) )
    {
        m_fgDynamicCCM = fgEnable;
    }
     return MTRUE;
}

MBOOL
Paramctrl::
enableDynamicShading(MBOOL fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[+enableDynamicShading](old, new)=(%d, %d)", m_fgDynamicShading, fgEnable);
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_eOperMode == EOperMode_Meta)
    {
        fgEnable = MFALSE;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgEnable(%d) m_eOperMode(%d)", __FUNCTION__, fgEnable, m_eOperMode);
    if ( checkParamChange(m_fgDynamicShading, fgEnable) )
    {
        m_fgDynamicShading = fgEnable;
    }

     return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspProfile(EIspProfile_T const eIspProfile)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.rMapping_Info.eIspProfile, eIspProfile) )
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+setIspProfile](old, new)=(%d, %d)", m_rIspCamInfo.rMapping_Info.eIspProfile, eIspProfile);
        m_rIspCamInfo.rMapping_Info.eIspProfile = eIspProfile;
    }
    std::string _str_factor_value = "MTKCam";
    MBOOL ret = NSCam::ATMsEventMgr::getInstance()->getFactor("APP_NAME", _str_factor_value);
    m_rIspCamInfo.rMapping_Info.eApp = (EApp_T)m_pIspTuningCustom->map_AppName_to_MappingInfo(_str_factor_value.c_str());

    return  MERR_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setRequestNumber(MINT32 const i4RequestNumber)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD_IF(m_bDebugEnable, "[+setRequestNumber](old, new)=(%d, %d)", m_i4RequestNum, i4RequestNumber);
    m_i4RequestNum = i4RequestNumber;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setSceneMode(EIndex_Scene_T const eScene)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eIdx_Scene, eScene) )
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+setSceneMode] scene(old, new)=(%d, %d)", m_rIspCamInfo.eIdx_Scene, eScene);
        m_rIspCamInfo.eIdx_Scene = eScene;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setEffect(EIndex_Effect_T const eEffect)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eIdx_Effect, eEffect) )
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+setEffect] effect(old, new)=(%d, %d)", m_rIspCamInfo.eIdx_Effect, eEffect);
        m_rIspCamInfo.eIdx_Effect = eEffect;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setOperMode(EOperMode_T const eOperMode)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_eOperMode, eOperMode) )
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+setOperMode](old, new)=(%d, %d)", m_eOperMode, eOperMode);
        m_eOperMode = eOperMode;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setSensorMode(ESensorMode_T const eSensorMode)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.rMapping_Info.eSensorMode, eSensorMode) )
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+setSensorMode](old, new)=(%d, %d)", m_rIspCamInfo.rMapping_Info.eSensorMode, eSensorMode);
        m_rIspCamInfo.rMapping_Info.eSensorMode = eSensorMode;
    }

    return  MERR_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setRawPath(MUINT32 const u4RawPath)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    m_ERawPath = static_cast<NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM>(u4RawPath);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setRawSize(MBOOL const fgBin, NSCam::MSize const RawSize)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if(fgBin){
        m_rIspCamInfo.rMapping_Info.eFrontBin = EFrontBin_Yes;

        m_rIspCamInfo.rCropRzInfo.fgFBinOnOff = MTRUE;
    }
    else{
        m_rIspCamInfo.rMapping_Info.eFrontBin = EFrontBin_No;

        m_rIspCamInfo.rCropRzInfo.fgFBinOnOff = MFALSE;
    }

    m_rIspCamInfo.rCropRzInfo.sRRZin.w = RawSize.w;
    m_rIspCamInfo.rCropRzInfo.sRRZin.h = RawSize.h;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setZoomRatio(MINT32 const i4ZoomRatio_x100)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.i4ZoomRatio_x100, i4ZoomRatio_x100) )
    {
        m_rIspCamInfo.rMapping_Info.eZoom_Idx = map_Zoom_value2index(i4ZoomRatio_x100);
        CAM_LOGD_IF(m_bDebugEnable, "[+setZoomRatio](old, new)=(%d, %d)", m_rIspCamInfo.i4ZoomRatio_x100, i4ZoomRatio_x100);
        m_rIspCamInfo.i4ZoomRatio_x100 = i4ZoomRatio_x100;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAWBInfo2ISP(AWB_ISP_INFO_T  const &rAWBInfo)
{
    MBOOL bAWBGainChanged = MFALSE;

    std::lock_guard<std::mutex> lock(m_Lock);

    ECT_T const eIdx_CT = map_CT_value2index(rAWBInfo.i4CCT);

    // CT index
    if (checkParamChange(m_rIspCamInfo.rMapping_Info.eCT_Idx, eIdx_CT)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.eIdx_CT](old, new)=(%d, %d)", m_rIspCamInfo.rMapping_Info.eCT_Idx, eIdx_CT);
         m_rIspCamInfo.rMapping_Info.eCT_Idx = eIdx_CT;
    }

    m_rIspCamInfo.rAWBInfo = rAWBInfo;

    get_CTIdx_neighbor(m_rIspCamInfo, m_rIspCamInfo.rMapping_Info.eCT_Idx, m_rIspCamInfo.rAWBInfo.i4CCT);

    return  MERR_OK;
}


MVOID
Paramctrl::
get_CTIdx_neighbor(RAWIspCamInfo& rIspCamInfo, ECT_T const CTIdx, MUINT32 const u4CT)
{
      MINT32 length = (MUINT32)m_rIspParam.ISPRegs.Ct_Env.u2Length;
      if (u4CT >= (MUINT32)m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[length-1]){
        rIspCamInfo.eIdx_CT_L = static_cast<ECT_T>(length-1);
        rIspCamInfo.eIdx_CT_U = static_cast<ECT_T>(length-1);
      }
      else if (CTIdx == 0){
        rIspCamInfo.eIdx_CT_L = static_cast<ECT_T>(0);
        rIspCamInfo.eIdx_CT_U = static_cast<ECT_T>(0);
      }
      else{
        rIspCamInfo.eIdx_CT_L = static_cast<ECT_T>(CTIdx - 1);
        rIspCamInfo.eIdx_CT_U = static_cast<ECT_T>(CTIdx);
      }
      CAM_LOGD_IF(m_bDebugEnable,"[%s](upIdx,lowerIdx, upCt, lowerCt) = (%d,%d,%d,%d)",
          __FUNCTION__,
          rIspCamInfo.eIdx_CT_U,
          rIspCamInfo.eIdx_CT_L,
          m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_U],
          m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_L]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspAEPreGain2(MINT32 /*i4SensorIndex*/, AWB_GAIN_T& rNewIspAEPreGain2)
{
    AWB_GAIN_T rCurrentIspAEPreGain2 = ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).getIspAEPreGain2();

    if (checkParamChange(rCurrentIspAEPreGain2.i4R, rNewIspAEPreGain2.i4R) ||
        checkParamChange(rCurrentIspAEPreGain2.i4G, rNewIspAEPreGain2.i4G) ||
        checkParamChange(rCurrentIspAEPreGain2.i4B, rNewIspAEPreGain2.i4B)) {
        ISP_MGR_AE_STAT_CONFIG_T::getInstance(m_eSensorDev).setIspAEPreGain2(rNewIspAEPreGain2);
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAEInfo2ISP(AE_ISP_INFO_T  const &rAEInfo_)
{
    CAM_LOGD_IF(m_bDebugEnable, "setAEInfo2ISP()");

    std::lock_guard<std::mutex> lock(m_Lock);

    AE_ISP_INFO_T rAEInfo = rAEInfo_;

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.aeinfo.ctrl", value, "0"); // 0: default, 1: bypass AE info
        MINT32 ctrl = atoi(value);
        if (ctrl) return  MERR_OK;
    }

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.aeinfo.iso", value, "-1"); // -1: default, others: set ISO value
        MINT32 iso = atoi(value);
        if (iso != -1) rAEInfo.u4P1RealISOValue = iso;
    }

    ELV_T const eIdx_Lv = map_LV_value2index(rAEInfo.i4RealLightValue_x10);
    m_rIspCamInfo.rMapping_Info.eLV_Idx = eIdx_Lv;
    get_LVIdx_neighbor(m_rIspCamInfo, eIdx_Lv, rAEInfo.i4RealLightValue_x10);
    CAM_LOGD_IF(m_bDebugEnable, "(rAEInfo.i4RealLightValue_x10, eIdx_Lv)=(%d, %d)", rAEInfo.i4RealLightValue_x10, eIdx_Lv);



    for(MINT32 group_index = 0; group_index< NVRAM_ISP_REGS_ISO_GROUP_NUM; group_index++){
        EISO_T eIdx_ISO = map_ISO_value2index(rAEInfo.u4P1RealISOValue, (EISO_GROUP_T)group_index);
    // ISO index
        if (checkParamChange(m_rIspCamInfo.rMapping_Info.eISO_Idx[group_index], eIdx_ISO)) {
            CAM_LOGD_IF(m_bDebugEnable,"[+m_rIspCamInfo.eIdx_ISO][%d](old, new)=(%d, %d)", group_index, m_rIspCamInfo.rMapping_Info.eISO_Idx[group_index], eIdx_ISO);
            m_rIspCamInfo.rMapping_Info.eISO_Idx[group_index] = eIdx_ISO;
            get_ISOIdx_neighbor(m_rIspCamInfo, m_rIspCamInfo.rMapping_Info.eISO_Idx[group_index], m_rIspCamInfo.rAEInfo.u4P1RealISOValue, (EISO_GROUP_T)group_index);
            CAM_LOGD_IF(m_bDebugEnable,"[+m_rIspCamInfo.eIdx_ISO][%d](upper, lower)=(%d, %d)", group_index, m_rIspCamInfo.eIdx_ISO_U[group_index], m_rIspCamInfo.eIdx_ISO_L[group_index]);
        }
    }

    CAM_LOGD_IF(1, "[+m_rIspCamInfo.rAEInfo.u4IspGain](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4P1DGNGain, rAEInfo.u4P1DGNGain);

    m_rIspCamInfo.rAEInfo = rAEInfo;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setCCUInfo2ISP(ISP_CCU_RESULT_T const &rCCUInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "setCCUInfo2ISP()");

    m_rIspCamInfo.rCCU_Result=rCCUInfo;
    m_rIspCamInfo.rCCU_Result.LTM.LTM_Valid=CCU_VALID_LAST;
    m_rIspCamInfo.rCCU_Result.HLR.HLR_Valid=CCU_VALID_LAST; // set p1 caminfo alway LAST

    return  MERR_OK;
}

const RAWIspCamInfo&
Paramctrl::
getCamInfo() {
        return m_rIspCamInfo;
}

EISO_T
Paramctrl::
map_ISO_value2index(MUINT32 const u4Iso, EISO_GROUP_T u4ISO_Group) const
{
    MUINT16 length = m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].u2Length;

    for(int i=0; i < length; i++){
        if(u4Iso <= m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].IDX_Partition[i])
            return static_cast<EISO_T>(i);
    }

    return static_cast<EISO_T>(length-1);  //max
}


EZoom_T
Paramctrl::
map_Zoom_value2index(MUINT32 const i4ZoomRatio_x100) const
{
    MUINT16 length = m_rIspParam.ISPRegs.Zoom_Env.u2Length;

    for(int i=0; i < length; i++){
        if(i4ZoomRatio_x100 <= m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[i])
            return static_cast<EZoom_T>(i);
    }

    return static_cast<EZoom_T>(length-1);  //max
}

ELV_T
Paramctrl::
map_LV_value2index(MINT32 const u4Lv) const
{
    MUINT16 length = m_rIspParam.ISPRegs.Lv_Env.u2Length;

    for(int i=0; i < length; i++){
        if(u4Lv <= m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[i])
            return static_cast<ELV_T>(i);
    }
    return static_cast<ELV_T>(length-1);  //max
}

MVOID
Paramctrl::
get_LVIdx_neighbor(RAWIspCamInfo& rIspCamInfo, ELV_T const LVIdx, MINT32 const i4LV)
{

      if (i4LV >= (ELV_T)m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[m_rIspParam.ISPRegs.Lv_Env.u2Length-1]){
        rIspCamInfo.eIdx_LV_L = static_cast<ELV_T>(m_rIspParam.ISPRegs.Lv_Env.u2Length-1);
        rIspCamInfo.eIdx_LV_U = static_cast<ELV_T>(m_rIspParam.ISPRegs.Lv_Env.u2Length-1);
      }
      else if (LVIdx == 0){
        rIspCamInfo.eIdx_LV_L = static_cast<ELV_T>(0);
        rIspCamInfo.eIdx_LV_U = static_cast<ELV_T>(0);
      }
      else{
        rIspCamInfo.eIdx_LV_L = static_cast<ELV_T>(LVIdx - 1);
        rIspCamInfo.eIdx_LV_U = static_cast<ELV_T>(LVIdx);
      }
      CAM_LOGD_IF(m_bDebugEnable,"[%s](upIdx,lowerIdx, upLv, lowerLv) = (%d,%d,%d,%d)",
          __FUNCTION__,
          rIspCamInfo.eIdx_LV_U,
          rIspCamInfo.eIdx_LV_L,
          m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
          m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L]);
}

ECT_T
Paramctrl::
map_CT_value2index(MINT32 const u4Ct) const
{
    MUINT16 length = m_rIspParam.ISPRegs.Ct_Env.u2Length;

    for(int i=0; i < length; i++){
        if(u4Ct < m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[i])
            return static_cast<ECT_T>(i);
    }
    return static_cast<ECT_T>(length-1);  //max
}

MUINT32
Paramctrl::
map_ISO_index2value(EISO_T const eIdx, EISO_GROUP_T u4ISO_Group) const
{
    EISO_T tIdx = eIdx;

    if(tIdx>(m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].u2Length-1))
        tIdx = (EISO_T)(m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].u2Length-1);

    return (MUINT32)m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].IDX_Partition[tIdx];
}


MVOID
Paramctrl::
get_ISOIdx_neighbor(RAWIspCamInfo& rIspCamInfo, EISO_T const IsoIdx, MUINT32 const u4Iso, EISO_GROUP_T u4ISO_Group)
{

      if (u4Iso >= (EISO_T)m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].IDX_Partition[m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].u2Length-1]){
        rIspCamInfo.eIdx_ISO_L[u4ISO_Group] = static_cast<EISO_T>(m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].u2Length - 1);
        rIspCamInfo.eIdx_ISO_U[u4ISO_Group] = static_cast<EISO_T>(m_rIspParam.ISPRegs.Iso_Env[u4ISO_Group].u2Length - 1);
      }
      else if (IsoIdx == 0){
        rIspCamInfo.eIdx_ISO_L[u4ISO_Group] = static_cast<EISO_T>(0);
        rIspCamInfo.eIdx_ISO_U[u4ISO_Group] = static_cast<EISO_T>(0);
      }
      else{
        rIspCamInfo.eIdx_ISO_L[u4ISO_Group] = static_cast<EISO_T>(IsoIdx - 1);
        rIspCamInfo.eIdx_ISO_U[u4ISO_Group] = IsoIdx;
      }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFlashInfo(MUINT32 const u4FlashMappingInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "setFlashInfo()");

    std::lock_guard<std::mutex> lock(m_Lock);

    m_rIspCamInfo.rMapping_Info.eFlash = (const EFlash_T)u4FlashMappingInfo;

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setIndex_Shading(MINT32 const i4IDX)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] idx %d", __FUNCTION__, i4IDX);

    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_pLscMgr) {
        m_pLscMgr->setCTIdx(i4IDX);
    } else {
        CAM_LOGD_IF(m_bDebugEnable, "[%s] m_pLscMgr is NULL", __FUNCTION__);
    }

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
getIndex_Shading(MVOID*const pCmdArg)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] idx ", __FUNCTION__);

    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_pLscMgr) {
        *(MINT8*)pCmdArg = m_pLscMgr->getCTIdx();
    } else {
        CAM_LOGD_IF(m_bDebugEnable, "[%s] m_pLscMgr is NULL", __FUNCTION__);
    }

    return  MERR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Since OBC mixed with AE info, so it is required to backup OBC info
// for dynamic bypass
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MERROR_ENUM
Paramctrl::
setPureOBCInfo(const ISP_NVRAM_OBC_T *pOBCInfo)
{
    memcpy(&m_backup_OBCInfo, pOBCInfo, sizeof(ISP_NVRAM_OBC_T));

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MERROR_ENUM
Paramctrl::
setRRZInfo(const ISP_NVRAM_RRZ_T *pRRZInfo)
{
    memcpy(&m_RRZInfo, pRRZInfo, sizeof(ISP_NVRAM_RRZ_T));
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getPureOBCInfo(ISP_NVRAM_OBC_T *pOBCInfo)
{
    memcpy(pOBCInfo, &m_backup_OBCInfo, sizeof(ISP_NVRAM_OBC_T));

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTGInfo(MINT32 const i4TGInfo, MINT32 const i4Width, MINT32 const i4Height)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] i4TGInfo(%d) i4Width(%d), i4Height(%d)", __FUNCTION__, i4TGInfo, i4Width, i4Height);

    std::lock_guard<std::mutex> lock(m_Lock);

    m_rIspCamInfo.rCropRzInfo.sTGout.w = i4Width;
    m_rIspCamInfo.rCropRzInfo.sTGout.h = i4Height;

    switch (i4TGInfo)
    {
    case CAM_TG_1:
        m_eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        m_eSensorTG = ESensorTG_2;
        break;
    case CAM_TG_3:
        m_eSensorTG = ESensorTG_3;
        break;
    default:
        CAM_LOGE("i4TGInfo = %d", i4TGInfo);
        return MERR_BAD_PARAM;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFDEnable(MBOOL const FD_enable)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] FDEnalbe = %d", __FUNCTION__, FD_enable);

    std::lock_guard<std::mutex> lock(m_Lock);

    if(FD_enable){
        m_rIspCamInfo.fgFDEnable = MTRUE;
    }
    else{
        m_rIspCamInfo.fgFDEnable = MFALSE;
    }
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFDInfo(CAMERA_TUNING_FD_INFO_T const FD_Info)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] FD_ratio = %f", __FUNCTION__, FD_Info.FDRatio);

    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_rIspCamInfo.fgFDEnable){
        MINT32 i = 0, j=0;
        memcpy(&(m_rIspCamInfo.rFdInfo), &FD_Info, sizeof(CAMERA_TUNING_FD_INFO_T));
        for(i = 0; i < m_rIspCamInfo.rFdInfo.FaceNum; i++) {
            CAM_LOGD_IF(m_bDebugEnable, "[%s] fld_GenderLabel: %d ",
                __FUNCTION__, m_rIspCamInfo.rFdInfo.fld_GenderLabel[i]);

            for(j = 0; j < 5; j++) {
                CAM_LOGD_IF(m_bDebugEnable, "[%s] faceNum(%d), m_rIspCamInfo.rFdInfo.YUVsts[%d][%d]: %d ",
                    __FUNCTION__, m_rIspCamInfo.rFdInfo.FaceNum, i, j, m_rIspCamInfo.rFdInfo.YUVsts[i][j]);
            }
        }
    }
    else{
        memset(&(m_rIspCamInfo.rFdInfo), 0, sizeof(CAMERA_TUNING_FD_INFO_T));
    }

    if(m_rIspCamInfo.rFdInfo.FaceNum != 0){
        m_rIspCamInfo.rMapping_Info.eFaceDetection = EFaceDetection_Yes;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] EFaceDetection_Yes",__FUNCTION__);
    }
    else{
        m_rIspCamInfo.rMapping_Info.eFaceDetection = EFaceDetection_No;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] EFaceDetection_No",__FUNCTION__);
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                            MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                            MFLOAT& M31, MFLOAT& M32, MFLOAT& M33)
{
    ISP_CCM_T rCCM;

    m_pCcmMgr->getCCM(rCCM);

    M11 = static_cast<MFLOAT>(rCCM.M11) / 512;
    M12 = static_cast<MFLOAT>(rCCM.M12) / 512;
    M13 = static_cast<MFLOAT>(rCCM.M13) / 512;
    M21 = static_cast<MFLOAT>(rCCM.M21) / 512;
    M22 = static_cast<MFLOAT>(rCCM.M22) / 512;
    M23 = static_cast<MFLOAT>(rCCM.M23) / 512;
    M31 = static_cast<MFLOAT>(rCCM.M31) / 512;
    M32 = static_cast<MFLOAT>(rCCM.M32) / 512;
    M33 = static_cast<MFLOAT>(rCCM.M33) / 512;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                            MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                            MFLOAT& M31, MFLOAT& M32, MFLOAT& M33)
{
    ISP_CCM_T rCCM;

    rCCM.M11 = static_cast<MINT32>((M11 * 512) + 0.5);
    rCCM.M12 = static_cast<MINT32>((M12 * 512) + 0.5);
    rCCM.M13 = static_cast<MINT32>((M13 * 512) + 0.5);
    rCCM.M21 = static_cast<MINT32>((M21 * 512) + 0.5);
    rCCM.M22 = static_cast<MINT32>((M22 * 512) + 0.5);
    rCCM.M23 = static_cast<MINT32>((M23 * 512) + 0.5);
    rCCM.M31 = static_cast<MINT32>((M31 * 512) + 0.5);
    rCCM.M32 = static_cast<MINT32>((M32 * 512) + 0.5);
    rCCM.M33 = static_cast<MINT32>((M33 * 512) + 0.5);

    m_pCcmMgr->setColorCorrectionTransform(rCCM);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setColorCorrectionMode(MINT32 i4ColorCorrectionMode)
{
    CAM_LOGD_IF(m_bDebugEnable, "%s() %d\n", __FUNCTION__, i4ColorCorrectionMode);

    switch (i4ColorCorrectionMode)
    {
    case MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX:
         m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX;
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX);
         m_rIspCamInfo.eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX;
         break;
    case MTK_COLOR_CORRECTION_MODE_FAST:
         m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_FAST;
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST);
         m_rIspCamInfo.eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_FAST;
         break;
    case MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY:
         m_eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY;
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY);
         m_rIspCamInfo.eColorCorrectionMode = MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY;
         break;
    default:
         CAM_LOGE("Incorrect color correction mode = %d", i4ColorCorrectionMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setEdgeMode(MINT32 i4EdgeMode)
{
    CAM_LOGD_IF(m_bDebugEnable, "%s()\n", __FUNCTION__);

    switch (i4EdgeMode)
    {
    case MTK_EDGE_MODE_OFF:
         m_rIspCamInfo.eEdgeMode = MTK_EDGE_MODE_OFF;
         break;
    case MTK_EDGE_MODE_FAST:
         m_rIspCamInfo.eEdgeMode = MTK_EDGE_MODE_FAST;
         break;
    case MTK_EDGE_MODE_HIGH_QUALITY:
         m_rIspCamInfo.eEdgeMode = MTK_EDGE_MODE_HIGH_QUALITY;
         break;
    case MTK_EDGE_MODE_ZERO_SHUTTER_LAG:
         m_rIspCamInfo.eEdgeMode = MTK_EDGE_MODE_ZERO_SHUTTER_LAG;
         break;
    default:
         CAM_LOGE("Incorrect edge mode = %d", i4EdgeMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
getIspGamma(NS3Av3::S3ACtrl_GetIspGamma* pParams) const
{
#if 0
//Chooo
    if  ( ! pParams ) {
        CAM_LOGE("bad GetIspGamma params");
        return MFALSE;
    }

    pParams->enabled = !(ISP_MGR_GGM_T::getInstance(m_eSensorDev).isLNREnable());
    pParams->gamma_lut_size = GGM_LUT_SIZE;
    if  ( ! pParams->gamma_lut ) {
        //linux style: users just want to query information in case of null buffer.
        return MTRUE;
    }

    MUINT32* pIspGamma = pParams->gamma_lut;
    ISP_NVRAM_GGM_T ggm;
    ISP_MGR_GGM_T::getInstance(m_eSensorDev).get(ggm);
    for (int i=0; i<GGM_LUT_SIZE; i++)
    {
        pIspGamma[i] = ggm.lut.lut[i].GGM_G;
        if (!m_bDebugEnable) continue;

        if ((i%10 == 0) && (i != 140))
            CAM_LOGD_IF(m_bDebugEnable, "[Paramctrl::getIspGamma] GGM_G[%d-%d](%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
                i, i+9,
                pIspGamma[i], pIspGamma[i+1], pIspGamma[i+2], pIspGamma[i+3], pIspGamma[i+4],
                pIspGamma[i+5], pIspGamma[i+6], pIspGamma[i+7], pIspGamma[i+8], pIspGamma[i+9]);
        else if (i == 140)
            CAM_LOGD_IF(m_bDebugEnable, "[Paramctrl::getIspGamma] GGM_G[%d-%d](%d, %d, %d, %d)",
                i, i+3,
                pIspGamma[i], pIspGamma[i+1], pIspGamma[i+2], pIspGamma[i+3]);

    }
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MERROR_ENUM
Paramctrl::
setNoiseReductionMode(MINT32 i4NRMode)
{
    CAM_LOGD_IF(m_bDebugEnable, "%s()\n", __FUNCTION__);

    switch (i4NRMode)
    {
    case MTK_NOISE_REDUCTION_MODE_OFF:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_OFF;
         break;
    case MTK_NOISE_REDUCTION_MODE_FAST:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_FAST;
         break;
    case MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY;
         break;
    case MTK_NOISE_REDUCTION_MODE_MINIMAL:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_MINIMAL;
         break;
    case MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG:
         m_rIspCamInfo.eNRMode = MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG;
         break;
    default:
         CAM_LOGE("Incorrect noise reduction mode = %d", i4NRMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setToneMapMode(MINT32 i4ToneMapMode)
{
    CAM_LOGD_IF(m_bDebugEnable, "%s()\n", __FUNCTION__);

    switch (i4ToneMapMode)
    {
    case MTK_TONEMAP_MODE_CONTRAST_CURVE:
         m_rIspCamInfo.eToneMapMode = MTK_TONEMAP_MODE_CONTRAST_CURVE;
         break;
    case MTK_TONEMAP_MODE_FAST:
         m_rIspCamInfo.eToneMapMode = MTK_TONEMAP_MODE_FAST;
         break;
    case MTK_TONEMAP_MODE_HIGH_QUALITY:
         m_rIspCamInfo.eToneMapMode = MTK_TONEMAP_MODE_HIGH_QUALITY;
         break;
    default:
         CAM_LOGE("Incorrect tone map mode = %d", i4ToneMapMode);
    }

    return MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getTonemapCurve_Red(MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getTonemapCurve_Green(MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getTonemapCurve_Blue(MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTonemapCurve_Red(MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTonemapCurve_Green(MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setTonemapCurve_Blue(MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum)
{
    m_pGgmMgr->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
convertPtPairsToGMA(const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA)
{
    return m_pGgmMgr->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
getP1ExifSizeInfo(MUINT32& u4RegCount,MUINT32& u4StartRegCount) const{
    MUINT32 u4RegCnt_start = (_A_P1(CAMCTL_R1_CAMCTL_EN_CTL) / 4);
    MUINT32 u4RegCnt_end = (_A_P1(LSCI_R1_LSCI_CON4) / 4);

    u4RegCount = u4RegCnt_end - u4RegCnt_start + 1;
    u4StartRegCount = u4RegCnt_start;
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
Paramctrl::
getP2ExifSizeInfo(MUINT32& u4RegCount,MUINT32& u4StartRegCount) const{
    MUINT32 u4RegCnt_start = (_A_P2(DIPCTL_D1A_DIPCTL_START) / 4);
    MUINT32 u4RegCnt_end = (_A_P2(LPCNR_D1A_LPCNR_STATUS) / 4);

    u4RegCount = u4RegCnt_end - u4RegCnt_start + 1;
    u4StartRegCount = u4RegCnt_start;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::setFdGamma(MINT32 i4Index, MINT32* fdGammaCurve, MUINT32 u4size){
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Paramctrl::
sendIspTuningIOCtrl(MUINT32 ctrl, MINTPTR arg1, MINTPTR arg2)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] (ctrl, arg1, arg2) = (%d, %x, %x)", __FUNCTION__, ctrl, (MUINT32)arg1, (MUINT32)arg2);

    IspTuningMgr::E_ISPTUNING_CTRL eCtrl = static_cast<IspTuningMgr::E_ISPTUNING_CTRL>(ctrl);

    switch (eCtrl)
    {
    case IspTuningMgr::E_ISPTUNING_NOTIFY_START:
        m_pGmaMgr->start();
        m_pLceMgr->start();
        m_pDceMgr->start();
        break;
    case IspTuningMgr::E_ISPTUNING_NOTIFY_CCU_START:
        if(m_pCcuIsp){
            if(m_pCcuIsp->init(m_i4SensorIdx, (ESensorDev_T)m_eSensorDev) != NSCcuIf::CCU_CTRL_SUCCEED){
               CAM_LOGE("m_pCcuIsp initialize fail");
            }
            struct ccu_ltm_init_data_in CCU_initData = {0};
            CCU_initData.ltm_nvram_addr = &(m_rIspParam.ISPToneMap.LTM[0]);
            CCU_initData.ltm_nvram_size = ISP_NVRAM_LTM_TBL_NUM * sizeof(ISP_NVRAM_LTM_TUNING_PARAM_T);
            CCU_initData.ltm_nvram_num = ISP_NVRAM_LTM_TBL_NUM;
            CCU_initData.hlr_nvram_addr = &(m_rIspParam.ISPToneMap.HLR[0]);
            CCU_initData.hlr_nvram_size = ISP_NVRAM_HLR_TBL_NUM * sizeof(ISP_NVRAM_HLR_TUNING_PARAM_T);
            CCU_initData.hlr_nvram_num = ISP_NVRAM_HLR_TBL_NUM;

            char value[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("vendor.isp.ltm_r1.disable", value, "0"); // 0: enable, 1: disable
            CCU_initData.ltm_en = !(atoi(value));
            property_get("vendor.isp.hlr_r1.disable", value, "0"); // 0: enable, 1: disable
            CCU_initData.hlr_en = !(atoi(value));

            m_pCcuIsp->ccuControl(MSG_TO_CCU_LTM_INIT, &CCU_initData, NULL);
            m_pCcuIsp->ccuControl(MSG_TO_CCU_LTM_START, NULL, NULL);
        }
        else{
            CAM_LOGE("m_pCcuIsp is NULL");
        }
        break;
    case IspTuningMgr::E_ISPTUNING_NOTIFY_STOP:
        m_pGmaMgr->stop();
        m_pLceMgr->stop();
        m_pDceMgr->stop();
        if(m_pCcuIsp) {
            m_pCcuIsp->ccuControl(MSG_TO_CCU_LTM_STOP, NULL, NULL);
            m_pCcuIsp->uninit();
        }
        break;
    case IspTuningMgr::E_ISPTUNING_GET_ISP_GAMMA:
        getIspGamma(reinterpret_cast<NS3Av3::S3ACtrl_GetIspGamma*>(arg1));
        break;
    case IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE:
        {
            m_u4RawFmtType = arg1;
#if 0
            if(m_u4RawFmtType == SENSOR_RAW_PD){
                ISP_MGR_BNR2_T::getInstance(m_eSensorDev).setPDSensorInfo((arg1 == SENSOR_RAW_PD ? MTRUE : MFALSE));

                CAMERA_BPCI_STRUCT* buf = NULL;
                int err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_PDC_TABLE, m_eSensorDev, (void*&)buf, 1);

                if(!err)
                {
                    ISP_MGR_BNR2_T::getInstance(m_eSensorDev).setBPCIBuf(buf);
                }
                else{
                    CAM_LOGE("No BPCI Table");
                }
            }
#endif
        }
        break;
    case IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO:
        break;
    case IspTuningMgr::E_ISPTUNING_UPDATE_OBC_INFO:
        //Chooo
        //prepareHw_PerFrame_OBC();
        break;
    case IspTuningMgr::E_ISPTUNING_GET_MFB_SIZE:
        {
            *(reinterpret_cast<MUINT32*>(arg1)) = sizeof(mfb_reg_t);
            break;
        }
    case IspTuningMgr::E_ISPTUNING_GET_LTM_CURVE_SIZE:
        {
            *(reinterpret_cast<MUINT32*>(arg1)) = LTM_CURVE_SIZE;
            *(reinterpret_cast<MUINT32*>(arg2)) = _A_P2(LTMTC_D1A_LTMTC_CURVE[0])/4;
            break;
        }
    case IspTuningMgr::E_ISPTUNING_GET_P2_BUFFER_SIZE:
        {
            MUINT32 u4RegCnt_start = 0;
            MUINT32 u4RegCnt = 0;
            getP2ExifSizeInfo(u4RegCnt, u4RegCnt_start);
            *(reinterpret_cast<MUINT32*>(arg1)) = u4RegCnt;
            *(reinterpret_cast<MUINT32*>(arg2)) = u4RegCnt_start;
            break;
        }
    case IspTuningMgr::E_ISPTUNING_NOTIFY_CONTROL_MODE:
        {
            MUINT8 u1ControlMode = *(reinterpret_cast<MUINT8*>(arg1));
            m_rIspCamInfo.eControlMode = (mtk_camera_metadata_enum_android_control_mode_t)u1ControlMode;
            CAM_LOGD_IF(m_rIspCamInfo.eControlMode != (mtk_camera_metadata_enum_android_control_mode_t)u1ControlMode , "[%s] MTK_CONTROL_MODE(%d -> %d)", __FUNCTION__, m_rIspCamInfo.eControlMode, (mtk_camera_metadata_enum_android_control_mode_t)u1ControlMode);
            break;
        }
    case IspTuningMgr::E_ISPTUNING_GET_LCE_GAIN:
        *(reinterpret_cast<MINT32*>(arg1)) = m_pLceMgr->getLastLCE_GainInfo();
        break;
    case IspTuningMgr::E_ISPTUNING_GetAINRParam:
        {
            CAM_IDX_QRY_COMB_WITH_ISO info_with_iso;
            RAWIspCamInfo tempInfo;
            MUINT16 u2Idx = 0, u2ScnIdx = 0, u2IsoGrpIdx = 0;
            IDXMGR_QUERY_RESULT result;

            if(arg1!=NULL){
                    NVRAM_CAMERA_FEATURE_STRUCT_t *pNvram = NULL;
                    int err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, m_eSensorDev, (void*&)pNvram);
                if (err!=0) {
                    CAM_LOGE("NvBufUtil getBufAndRead fail");
                } else {
                    MUINT16 idxL = 0, idxU = 0, i4ISO = 0;
                    memcpy(&(info_with_iso), reinterpret_cast<void*>(arg1), sizeof(CAM_IDX_QRY_COMB_WITH_ISO));
                    i4ISO = info_with_iso.i4ISO;

                    for(MINT32 group_index = 0; group_index< NVRAM_ISP_REGS_ISO_GROUP_NUM; group_index++){
                        EISO_T eIdx_ISO = map_ISO_value2index(info_with_iso.i4ISO, (EISO_GROUP_T)group_index);
                        get_ISOIdx_neighbor(tempInfo, eIdx_ISO, i4ISO, (EISO_GROUP_T)group_index);
                    }

                    memcpy((info_with_iso.mapping_info.eISO_Idx), tempInfo.eIdx_ISO_U, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
                    idxU = IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->query(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), EModule_AINR, info_with_iso.mapping_info, result, __FUNCTION__);
                    memcpy((info_with_iso.mapping_info.eISO_Idx), tempInfo.eIdx_ISO_L, sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);
                    idxL = IdxMgr::createInstance(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev))->query(static_cast<NSIspTuning::ESensorDev_T>(m_eSensorDev), EModule_AINR, info_with_iso.mapping_info, result, __FUNCTION__);

                    ALOGD("SmoothAINR_SW iso(%d),upper(%d),lower(%d)", i4ISO, idxU, idxL);
                    SmoothAINR_SW(i4ISO,  // Real ISO
                            m_rIspParam.ISPRegs.Iso_Env[result.group].IDX_Partition[tempInfo.eIdx_ISO_U[result.group]], // Upper ISO
                            m_rIspParam.ISPRegs.Iso_Env[result.group].IDX_Partition[tempInfo.eIdx_ISO_L[result.group]], // Lower ISO
                            pNvram->AINR[idxU], // settings for upper ISO
                            pNvram->AINR[idxL], // settings for lower ISO
                            *(reinterpret_cast<FEATURE_NVRAM_AINR_T*>(arg2)));  // Output
                }
            }else {
                ALOGE("E_ISPTUNING_GetAINRParam pointer is NULL");
            }
            break;
        }
    default:
        CAM_LOGE("%s: ctrl not supported (ctrl, arg1, arg2) = (%d, %x, %x)", __FUNCTION__, ctrl, (MUINT32)arg1, (MUINT32)arg2);
        break;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
Paramctrl::
setLCESOBuffer(void* lcesBuffer)
{
    m_pLCESBuffer = (IImageBuffer*) lcesBuffer;
}

MVOID
Paramctrl::
setDCESOBuffer(void* dcesBuffer)
{
    m_pDCESBuffer = (IImageBuffer*) dcesBuffer;
}

MVOID
Paramctrl::
setDualSyncInfo(void* pDualSynInfo)
{
    m_bSlave = MTRUE;
    ::memcpy(&m_ISP_Sync_Info, pDualSynInfo, sizeof(DUAL_ISP_SYNC_INFO_T));
}

MVOID
Paramctrl::
getDualSyncInfo(void* pDualSynInfo)
{
    ::memcpy(pDualSynInfo, &m_ISP_Sync_Info, sizeof(DUAL_ISP_SYNC_INFO_T));
}

MVOID
Paramctrl::
clearDualSyncInfo()
{
    m_bSlave = MFALSE;
    memset(&m_ISP_Sync_Info, 0, sizeof(DUAL_ISP_SYNC_INFO_T));
}

MBOOL
Paramctrl::
isIspInterpolation(const RAWIspCamInfo& rIspCamInfo)
{

    if (m_IspInterpCtrl &&
        m_pIspTuningCustom->is_to_invoke_isp_interpolation(rIspCamInfo))
    {
        MINT32 iso_diff = memcmp(&(rIspCamInfo.eIdx_ISO_L), &(rIspCamInfo.eIdx_ISO_U), sizeof(EISO_T)*NVRAM_ISP_REGS_ISO_GROUP_NUM);

        if (iso_diff)
        {
            return MTRUE;
        }
    }

    return MFALSE;
}



