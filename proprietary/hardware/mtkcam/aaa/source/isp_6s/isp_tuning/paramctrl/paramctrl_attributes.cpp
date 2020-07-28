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

#include "property_utils.h"
#include <isp_interpolation/isp_interpolation.h>

using namespace NSIspTuning;


#define _A_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

#define _A_P2(reg)\
    ((MUINT32)offsetof(dip_a_reg_t, reg))

MVOID
Paramctrl::
enableDynamicTuning(MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] (old, new)=(%d, %d)", __FUNCTION__, m_fgDynamicTuning, fgEnable);

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
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] (old, new)=(%d, %d)", __FUNCTION__, m_fgDynamicCCM, fgEnable);

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
    m_pLscMgr->enableDynamicShading(fgEnable, m_eOperMode);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setRequestNumber(MINT32 const i4RequestNumber)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] (old, new)=(%d, %d)", __FUNCTION__, m_i4RequestNum, i4RequestNumber);
    m_i4RequestNum = i4RequestNumber;

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
        CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] (old, new)=(%d, %d)", __FUNCTION__, m_eOperMode, eOperMode);
        m_eOperMode = eOperMode;
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

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] (old, new)=(%d, %d)", __FUNCTION__, u4RawPath, m_ERawPath);

    m_ERawPath = static_cast<NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM>(u4RawPath);

    return  MERR_OK;
}

MVOID
Paramctrl::
get_CTIdx_neighbor(RAWIspCamInfo& rIspCamInfo, ECT_T const CTIdx)
{
      MINT32 length = (MUINT32)m_rIspParam.ISPRegs.Ct_Env.u2Length;
      if (rIspCamInfo.rAWBInfo.i4CCT >= (MUINT32)m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[length-1]){
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
      CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE,"[%s] (upIdx, lowerIdx, upCt, lowerCt) = (%d, %d, %d, %d)",
          __FUNCTION__,
          rIspCamInfo.eIdx_CT_U,
          rIspCamInfo.eIdx_CT_L,
          m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_U],
          m_rIspParam.ISPRegs.Ct_Env.IDX_Partition[rIspCamInfo.eIdx_CT_L]);
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
get_LVIdx_neighbor(RAWIspCamInfo& rIspCamInfo, ELV_T const LVIdx)
{

      if (rIspCamInfo.rAEInfo.i4RealLightValue_x10 >= (ELV_T)m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[m_rIspParam.ISPRegs.Lv_Env.u2Length-1]){
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
      CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE,"[%s] (upIdx, lowerIdx, upLv, lowerLv) = (%d, %d, %d, %d)",
          __FUNCTION__,
          rIspCamInfo.eIdx_LV_U,
          rIspCamInfo.eIdx_LV_L,
          m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_U],
          m_rIspParam.ISPRegs.Lv_Env.IDX_Partition[rIspCamInfo.eIdx_LV_L]);
}

EIspLV_T
Paramctrl::
map_LV_value2IspLV_index(MINT32 const u4Lv) const
{
    MUINT16 length = m_rIspParam.ISPRegs.IspLv_Env.u2Length;

    for(int i=0; i < length; i++){
        if(u4Lv <= m_rIspParam.ISPRegs.IspLv_Env.IDX_Partition[i])
            return static_cast<EIspLV_T>(i);
    }
    return static_cast<EIspLV_T>(length-1);  //max
}

MVOID
Paramctrl::
get_IspLVIdx_neighbor(RAWIspCamInfo& rIspCamInfo, EIspLV_T const IspLVIdx)
{

      if (rIspCamInfo.rAEInfo.i4RealLightValue_x10 >= (EIspLV_T)m_rIspParam.ISPRegs.IspLv_Env.IDX_Partition[m_rIspParam.ISPRegs.IspLv_Env.u2Length-1]){
        rIspCamInfo.eIdx_IspLV_L = static_cast<EIspLV_T>(m_rIspParam.ISPRegs.IspLv_Env.u2Length-1);
        rIspCamInfo.eIdx_IspLV_U = static_cast<EIspLV_T>(m_rIspParam.ISPRegs.IspLv_Env.u2Length-1);
      }
      else if (IspLVIdx == 0){
        rIspCamInfo.eIdx_IspLV_L = static_cast<EIspLV_T>(0);
        rIspCamInfo.eIdx_IspLV_U = static_cast<EIspLV_T>(0);
      }
      else{
        rIspCamInfo.eIdx_IspLV_L = static_cast<EIspLV_T>(IspLVIdx - 1);
        rIspCamInfo.eIdx_IspLV_U = static_cast<EIspLV_T>(IspLVIdx);
      }
      CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE,"[%s] (upIdx, lowerIdx, upIspLv, lowerIspLv) = (%d, %d, %d, %d)",
          __FUNCTION__,
          rIspCamInfo.eIdx_IspLV_U,
          rIspCamInfo.eIdx_IspLV_L,
          m_rIspParam.ISPRegs.IspLv_Env.IDX_Partition[rIspCamInfo.eIdx_IspLV_U],
          m_rIspParam.ISPRegs.IspLv_Env.IDX_Partition[rIspCamInfo.eIdx_IspLV_L]);
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

MERROR_ENUM
Paramctrl::
setIndex_Shading(MINT32 const i4IDX)
{
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] idx %d", __FUNCTION__, i4IDX);

    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_pLscMgr) {
        m_pLscMgr->setCTIdx(i4IDX);
    } else {
        CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] m_pLscMgr is NULL", __FUNCTION__);
    }

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
getIndex_Shading(MVOID*const pCmdArg)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_pLscMgr) {
        *(MINT8*)pCmdArg = m_pLscMgr->getCTIdx();
    } else {
        CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] m_pLscMgr is NULL", __FUNCTION__);
    }

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] idx %d", __FUNCTION__, *(MINT8*)pCmdArg);

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
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "%s() %d\n", __FUNCTION__, i4ColorCorrectionMode);

    switch (i4ColorCorrectionMode)
    {
    case MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX:
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX);
         break;
    case MTK_COLOR_CORRECTION_MODE_FAST:
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_FAST);
         break;
    case MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY:
         m_pCcmMgr->setColorCorrectionMode(MTK_COLOR_CORRECTION_MODE_HIGH_QUALITY);
         break;
    default:
         CAM_LOGE("Incorrect color correction mode = %d", i4ColorCorrectionMode);
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
        if (!m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE) continue;

        if ((i%10 == 0) && (i != 140))
            CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[Paramctrl::getIspGamma] GGM_G[%d-%d](%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
                i, i+9,
                pIspGamma[i], pIspGamma[i+1], pIspGamma[i+2], pIspGamma[i+3], pIspGamma[i+4],
                pIspGamma[i+5], pIspGamma[i+6], pIspGamma[i+7], pIspGamma[i+8], pIspGamma[i+9]);
        else if (i == 140)
            CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[Paramctrl::getIspGamma] GGM_G[%d-%d](%d, %d, %d, %d)",
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
//HHHHH
    MUINT32 u4RegCnt_start = 0;//(_A_P2(DIPCTL_D1A_DIPCTL_START) / 4);
    MUINT32 u4RegCnt_end = 0; //(_A_P2(LPCNR_D1A_LPCNR_STATUS) / 4);

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
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_SET_ATTRIBUTE, "[%s] (ctrl, arg1, arg2) = (%d, %x, %x)", __FUNCTION__, ctrl, (MUINT32)arg1, (MUINT32)arg2);

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
            if(m_pCcuIsp->init(m_i4SensorIdx, m_eSensorDev) != NSCcuIf::CCU_CTRL_SUCCEED){
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
setLCESOBuffer(void* lcesBuffer, void* lceshoBuffer)
{
    m_pLCESBuffer = (IImageBuffer*) lcesBuffer;
    m_pLCESHOBuffer = (IImageBuffer*) lceshoBuffer;
}

MVOID
Paramctrl::
setDCESOBuffer(void* dcesBuffer)
{
    m_pDCESBuffer = (IImageBuffer*) dcesBuffer;
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



