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
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <ispfeature.h>
#include <ccm_mgr.h>
#include <gma_mgr.h>
#include <lce_mgr.h>
#include <lsc/ILscMgr.h>
#include <isp_mgr.h>
#include <mtkcam/drv/IHalSensor.h>
#include "paramctrl.h"
#include <isp_tuning_mgr.h>
//#include <librwb/MTKRWB.h>
#include <lib3a/isp_algo_common.h>
#include <nvbuf_util.h>
#include <nvram_drv.h>


using namespace android;
using namespace NSIspTuning;
using namespace NSIspTuningv3;

MVOID
Paramctrl::
enableDynamicTuning(MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[+enableDynamicTuning](old, new)=(%d, %d)", m_fgDynamicTuning, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicTuning, fgEnable) )
    {
        m_fgDynamicTuning = fgEnable;
    }
}


MVOID
Paramctrl::
enableDynamicCCM(MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[+enableDynamicCCM](old, new)=(%d, %d)", m_fgDynamicCCM, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicCCM, fgEnable) )
    {
        m_fgDynamicCCM = fgEnable;
    }
}

MVOID
Paramctrl::
enableDynamicShading(MBOOL fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[+enableDynamicShading](old, new)=(%d, %d)", m_fgDynamicShading, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if (m_eOperMode == EOperMode_Meta)
    {
        fgEnable = MFALSE;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[%s] fgEnable(%d) m_eOperMode(%d)", __FUNCTION__, fgEnable, m_eOperMode);
    if ( checkParamChange(m_fgDynamicShading, fgEnable) )
    {
        m_fgDynamicShading = fgEnable;
    }
}


#if 0
MVOID
Paramctrl::
updateShadingNVRAMdata(MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[+updateShadingNVRAMdata](old, new)=(%d, %d)", m_fgShadingNVRAMdataChange, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgShadingNVRAMdataChange, fgEnable) )
    {
        m_fgShadingNVRAMdataChange = fgEnable;
    }

    CAM_LOGD_IF(m_bDebugEnable, "[-updateShadingNVRAMdata] return");
    return;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspProfile(EIspProfile_T const eIspProfile)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eIspProfile, eIspProfile) )
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+setIspProfile](old, new)=(%d, %d)", m_rIspCamInfo.eIspProfile, eIspProfile);
        m_rIspCamInfo.eIspProfile = eIspProfile;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setSceneMode(EIndex_Scene_T const eScene)
{
    Mutex::Autolock lock(m_Lock);

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
    Mutex::Autolock lock(m_Lock);

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
    Mutex::Autolock lock(m_Lock);

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
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eSensorMode, eSensorMode) )
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+setSensorMode](old, new)=(%d, %d)", m_rIspCamInfo.eSensorMode, eSensorMode);
        m_rIspCamInfo.eSensorMode = eSensorMode;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFrontBinInfo(ISP_BIN_INFO_T const BinInfo)
{
    Mutex::Autolock lock(m_Lock);

    m_rIspCamInfo.BinInfo = BinInfo;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setZoomRatio(MINT32 const i4ZoomRatio_x100)
{
    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.i4ZoomRatio_x100, i4ZoomRatio_x100) )
    {
        m_rIspCamInfo.i4PrevZoomRatio_x100 = m_rIspCamInfo.i4ZoomRatio_x100;
        m_rIspCamInfo.eIdx_Zoom = map_Zoom_value2index(i4ZoomRatio_x100);
        CAM_LOGD_IF(m_bDebugEnable, "[+setZoomRatio](old, new)=(%d, %d)", m_rIspCamInfo.i4ZoomRatio_x100, i4ZoomRatio_x100);
        m_rIspCamInfo.i4ZoomRatio_x100 = i4ZoomRatio_x100;
        get_ZoomIdx_neighbor(m_rIspCamInfo, m_rIspCamInfo.eIdx_Zoom, i4ZoomRatio_x100);
    }

    return  MERR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAWBInfo(AWB_INFO_T const &rAWBInfo)
{
    MBOOL bAWBGainChanged = MFALSE;

    Mutex::Autolock lock(m_Lock);
#if 0
   if (checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4R, rAWBInfo.rCurrentAWBGain.i4R) ||
       checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4G, rAWBInfo.rCurrentAWBGain.i4G) ||
       checkParamChange(m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4B, rAWBInfo.rCurrentAWBGain.i4B)) {
        bAWBGainChanged = MTRUE;
        CAM_LOGD_IF(m_bDebugEnable, "setAWBInfo(): bAWBGainChanged = MTRUE");
   }
#endif

   EIndex_PCA_LUT_CT_T const eIdx_CT = map_CT_value2index(rAWBInfo.i4CCT);

   // CT index
   if (checkParamChange(m_rIspCamInfo.eIdx_CT, eIdx_CT)) {
       CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.eIdx_CT](old, new)=(%d, %d)", m_rIspCamInfo.eIdx_CT, eIdx_CT);
       m_rIspCamInfo.eIdx_CT = eIdx_CT;
   }

   m_rIspCamInfo.rAWBInfo = rAWBInfo;

   get_CTIdx_neighbor(m_rIspCamInfo, m_rIspCamInfo.eIdx_CT, m_rIspCamInfo.rAWBInfo.i4CCT);

   return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAWBGain(AWB_GAIN_T& /*rNewIspAWBGain*/)
{
#if 0
    CAM_LOGD_IF(m_bDebugEnable,"%s(): rNewIspAWBGain.i4R = %d, rNewIspAWBGain.i4G = %d, rNewIspAWBGain.i4B = %d\n",
              __FUNCTION__, rNewIspAWBGain.i4R, rNewIspAWBGain.i4G, rNewIspAWBGain.i4B);

    AWB_GAIN_T rCurrentIspAWBGain = ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspAWBGain();

   if (checkParamChange(rCurrentIspAWBGain.i4R, rNewIspAWBGain.i4R) ||
       checkParamChange(rCurrentIspAWBGain.i4G, rNewIspAWBGain.i4G) ||
       checkParamChange(rCurrentIspAWBGain.i4B, rNewIspAWBGain.i4B)) {
       ISP_MGR_RPG_T::getInstance(m_eSensorDev, m_eSensorTG).setIspAWBGain(rNewIspAWBGain);
       ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspAWBGain(rNewIspAWBGain);
   }
#endif

    return  MERR_OK;
}

EIndex_PCA_LUT_CT_T
Paramctrl::
map_CT_value2index(MUINT32 const u4CT) const
{
    MUINT32 t_low = 0, t_high = ISP_NVRAM_COLOR_CT_NUM - 1;

    MUINT32 t_u4CT = u4CT;
    MUINT32 t_ceiling =m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[t_high];

    if( t_u4CT >= t_ceiling){
           t_u4CT = t_ceiling - 1;
    }


    while ( (t_low <= t_high) && (t_high > 1) )
    {
        int t_mid = (t_low + t_high)/2;
        MUINT32 t_u4mid = m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[t_mid];

        if ( (t_u4CT < t_u4mid) && (t_u4CT >= (MUINT32)m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[t_mid-1]) )
        {
            return static_cast<EIndex_PCA_LUT_CT_T>(t_mid);
        }
        else if (t_u4CT >= t_u4mid)
        {
            t_low = t_mid + 1 ;
        }
        else if (t_u4CT < t_u4mid)
        {
            t_high = t_mid - 1 ;
        }
     }

    if (t_high < 2){
        if ( (t_u4CT < (MUINT32)m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[t_high]) && (t_u4CT >= (MUINT32)m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[t_low])){
            return static_cast<EIndex_PCA_LUT_CT_T>(t_high);
        }
        else{
            return static_cast<EIndex_PCA_LUT_CT_T>(t_low);
        }
    }

    CAM_LOGE("ISO_MAPPING_ERROR");
    return eIDX_PCA_CT_00;  //default
}

MVOID
Paramctrl::
get_CTIdx_neighbor(RAWIspCamInfo& rIspCamInfo, EIndex_PCA_LUT_CT_T const CTIdx, MUINT32 const u4CT)
{
      if (u4CT >= (MUINT32)m_rIspParam.ISPColorTbl.ISPColor.COLOR_CT_Env[ISP_NVRAM_COLOR_CT_NUM-1]){
        rIspCamInfo.eIdx_CT_L = static_cast<EIndex_PCA_LUT_CT_T>(ISP_NVRAM_COLOR_CT_NUM - 1);
        rIspCamInfo.eIdx_CT_U = static_cast<EIndex_PCA_LUT_CT_T>(ISP_NVRAM_COLOR_CT_NUM - 1);
      }
      else if (CTIdx == 0){
        rIspCamInfo.eIdx_CT_L = static_cast<EIndex_PCA_LUT_CT_T>(0);
        rIspCamInfo.eIdx_CT_U = static_cast<EIndex_PCA_LUT_CT_T>(0);
      }
      else{
        rIspCamInfo.eIdx_CT_L = static_cast<EIndex_PCA_LUT_CT_T>(CTIdx - 1);
        rIspCamInfo.eIdx_CT_U = CTIdx;

      }
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
setAEInfo(AE_INFO_T const &rAEInfo_)
{
    CAM_LOGD_IF(m_bDebugEnable, "setAEInfo()");

    Mutex::Autolock lock(m_Lock);

    AE_INFO_T rAEInfo = rAEInfo_;

    {
        if (m_i4AeinfoCtrl) return  MERR_OK;
    }

    {
        MINT32 iso = propertyGet("vendor.isp.aeinfo.iso", -1);
        if (iso != -1) rAEInfo.u4RealISOValue = iso;
    }

    // refine ISO value with LV value for ISP
    if (m_pIspTuningCustom->is_to_invoke_remap_ISP_ISO_with_LV(m_rIspCamInfo))
    {
        rAEInfo.u4RealISOValue = m_pIspTuningCustom->remap_ISP_ISO_with_LV(m_rIspParam.ISPRegs.Iso_Env, rAEInfo.i4LightValue_x10, rAEInfo.u4RealISOValue, 1);
    }

    EIndex_ISO_T const eIdx_ISO = map_ISO_value2index(rAEInfo.u4RealISOValue);

    // ISO value
    if (checkParamChange(m_rIspCamInfo.rAEInfo.u4RealISOValue, rAEInfo.u4RealISOValue)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.u4ISOValue](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4RealISOValue, rAEInfo.u4RealISOValue);
        //m_rIspCamInfo.rAEInfo.u4RealISOValue = rAEInfo.u4RealISOValue;
    }

    // ISO index
    if (checkParamChange(m_rIspCamInfo.eIdx_ISO, eIdx_ISO)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.eIdx_ISO](old, new)=(%d, %d)", m_rIspCamInfo.eIdx_ISO, eIdx_ISO);
        m_rIspCamInfo.eIdx_ISO = eIdx_ISO;
    }

    // AE Stable Cnt : need to before LV
    if(checkParamChange(m_rIspCamInfo.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt))
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4AEStableCnt](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt);
        //CAM_LOGD("[+m_rIspCamInfo.rAEInfo.u4AEStableCnt](old, new)=(%d, %d)\n", m_rIspCamInfo.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt);
        //m_rIspCamInfo.rAEInfo.u4AEStableCnt = rAEInfo.u4AEStableCnt;
    }

    // LV
    if (checkParamChange(m_rIspCamInfo.rAEInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.i4LightValue_x10](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10);
        //m_rIspCamInfo.rAEInfo.i4LightValue_x10 = rAEInfo.i4LightValue_x10;
    }

    // ISP gain
    if (checkParamChange(m_rIspCamInfo.rAEInfo.u4IspGain, rAEInfo.u4IspGain)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4IspGain](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4IspGain, rAEInfo.u4IspGain);
        //m_rIspCamInfo.rAEInfo.u4IspGain = rAEInfo.u4IspGain;
    }

    //check Flare offset
    if(checkParamChange(m_rIspCamInfo.rAEInfo.i2FlareOffset, rAEInfo.i2FlareOffset)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.i2FlareOffset](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i2FlareOffset, rAEInfo.i2FlareOffset);
        //m_rIspCamInfo.rAEInfo.i2FlareOffset = rAEInfo.i2FlareOffset;
    }

    //check Gamma Index
    if(checkParamChange(m_rIspCamInfo.rAEInfo.i4GammaIdx, rAEInfo.i4GammaIdx)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.i4GammaIdx](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i4GammaIdx, rAEInfo.i4GammaIdx);
        //m_rIspCamInfo.rAEInfo.i4GammaIdx = rAEInfo.i4GammaIdx;
    }

    if(checkParamChange(m_rIspCamInfo.rAEInfo.u4SWHDR_SE, rAEInfo.u4SWHDR_SE) || checkParamChange(m_rIspCamInfo.rAEInfo.i4LESE_Ratio, rAEInfo.i4LESE_Ratio)
                                                                              || checkParamChange(m_rIspCamInfo.rAEInfo.i4HdrSeg, rAEInfo.i4HdrSeg))
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4SWHDR_SE](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4SWHDR_SE, rAEInfo.u4SWHDR_SE);
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.i4LESE_Ratio](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i4LESE_Ratio, rAEInfo.i4LESE_Ratio);
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.i4HdrSeg](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.i4HdrSeg, rAEInfo.i4HdrSeg);
    }

    if(checkParamChange(m_rIspCamInfo.rAEInfo.u4MaxISO, rAEInfo.u4MaxISO)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfo.rAEInfo.u4MaxISO](old, new)=(%d, %d)", m_rIspCamInfo.rAEInfo.u4MaxISO, rAEInfo.u4MaxISO);
        //m_rIspCamInfo.rAEInfo.u4MaxISO = rAEInfo.u4MaxISO;
        // TBD
    }

    m_rIspCamInfo.rAEInfo = rAEInfo;
    get_ISOIdx_neighbor(m_rIspCamInfo, m_rIspCamInfo.eIdx_ISO, m_rIspCamInfo.rAEInfo.u4RealISOValue);
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAEInfoP2(AE_INFO_T const &rAEInfo_)
{
    CAM_LOGD_IF(m_bDebugEnable, "setAEInfoP2()");

    Mutex::Autolock lock(m_Lock);

    AE_INFO_T rAEInfo = rAEInfo_;

    {
        if (m_i4AeinfoCtrl) return  MERR_OK;
    }

    {
        MINT32 iso = propertyGet("vendor.isp.aeinfo.iso", -1);
        if (iso != -1) rAEInfo.u4RealISOValue = iso;
    }

    // refine ISO value with LV value for ISP
    if (m_pIspTuningCustom->is_to_invoke_remap_ISP_ISO_with_LV(m_rIspCamInfoP2))
    {
        rAEInfo.u4RealISOValue = m_pIspTuningCustom->remap_ISP_ISO_with_LV(m_rIspParam.ISPRegs.Iso_Env, rAEInfo.i4LightValue_x10, rAEInfo.u4RealISOValue, 1);
    }

    EIndex_ISO_T const eIdx_ISO = map_ISO_value2index(rAEInfo.u4RealISOValue);

    // ISO value
    if (checkParamChange(m_rIspCamInfoP2.rAEInfo.u4RealISOValue, rAEInfo.u4RealISOValue)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.u4ISOValue](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.u4RealISOValue, rAEInfo.u4RealISOValue);
        //m_rIspCamInfoP2.rAEInfo.u4RealISOValue = rAEInfo.u4RealISOValue;
    }

    // ISO index
    if (checkParamChange(m_rIspCamInfoP2.eIdx_ISO, eIdx_ISO)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.eIdx_ISO](old, new)=(%d, %d)", m_rIspCamInfoP2.eIdx_ISO, eIdx_ISO);
        m_rIspCamInfoP2.eIdx_ISO = eIdx_ISO;
    }

    // AE Stable Cnt : need to before LV
    if(checkParamChange(m_rIspCamInfoP2.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt))
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.u4AEStableCnt](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt);
        //CAM_LOGD("[+m_rIspCamInfoP2.rAEInfo.u4AEStableCnt](old, new)=(%d, %d)\n", m_rIspCamInfoP2.rAEInfo.u4AEStableCnt, rAEInfo.u4AEStableCnt);
        //m_rIspCamInfoP2.rAEInfo.u4AEStableCnt = rAEInfo.u4AEStableCnt;
    }

    // LV
    if (checkParamChange(m_rIspCamInfoP2.rAEInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.i4LightValue_x10](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.i4LightValue_x10, rAEInfo.i4LightValue_x10);
        //m_rIspCamInfoP2.rAEInfo.i4LightValue_x10 = rAEInfo.i4LightValue_x10;
    }

    // ISP gain
    if (checkParamChange(m_rIspCamInfoP2.rAEInfo.u4IspGain, rAEInfo.u4IspGain)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.u4IspGain](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.u4IspGain, rAEInfo.u4IspGain);
        //m_rIspCamInfoP2.rAEInfo.u4IspGain = rAEInfo.u4IspGain;
    }

    //check Flare offset
    if(checkParamChange(m_rIspCamInfoP2.rAEInfo.i2FlareOffset, rAEInfo.i2FlareOffset)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.i2FlareOffset](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.i2FlareOffset, rAEInfo.i2FlareOffset);
        //m_rIspCamInfoP2.rAEInfo.i2FlareOffset = rAEInfo.i2FlareOffset;
    }

    //check Gamma Index
    if(checkParamChange(m_rIspCamInfoP2.rAEInfo.i4GammaIdx, rAEInfo.i4GammaIdx)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.i4GammaIdx](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.i4GammaIdx, rAEInfo.i4GammaIdx);
        //m_rIspCamInfoP2.rAEInfo.i4GammaIdx = rAEInfo.i4GammaIdx;
    }

    if(checkParamChange(m_rIspCamInfoP2.rAEInfo.u4SWHDR_SE, rAEInfo.u4SWHDR_SE) || checkParamChange(m_rIspCamInfoP2.rAEInfo.i4LESE_Ratio, rAEInfo.i4LESE_Ratio)
                                                                              || checkParamChange(m_rIspCamInfoP2.rAEInfo.i4HdrSeg, rAEInfo.i4HdrSeg))
    {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.u4SWHDR_SE](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.u4SWHDR_SE, rAEInfo.u4SWHDR_SE);
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.i4LESE_Ratio](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.i4LESE_Ratio, rAEInfo.i4LESE_Ratio);
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.i4HdrSeg](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.i4HdrSeg, rAEInfo.i4HdrSeg);
    }

    if(checkParamChange(m_rIspCamInfoP2.rAEInfo.u4MaxISO, rAEInfo.u4MaxISO)) {
        CAM_LOGD_IF(m_bDebugEnable, "[+m_rIspCamInfoP2.rAEInfo.u4MaxISO](old, new)=(%d, %d)", m_rIspCamInfoP2.rAEInfo.u4MaxISO, rAEInfo.u4MaxISO);
        //m_rIspCamInfoP2.rAEInfo.u4MaxISO = rAEInfo.u4MaxISO;
        // TBD
    }

    get_ISOIdx_neighbor(m_rIspCamInfoP2, m_rIspCamInfoP2.eIdx_ISO, m_rIspCamInfoP2.rAEInfo.u4RealISOValue);

    m_rIspCamInfoP2.rAEInfo = rAEInfo;
    m_bSetAeInfoP2 = MTRUE;
    return  MERR_OK;
}
const RAWIspCamInfo&
Paramctrl::
getCamInfo() {
    if (m_bSetAeInfoP2) {
        RAWIspCamInfo convetInfo;
        memcpy(&convetInfo, &m_rIspCamInfo, sizeof(RAWIspCamInfo));
        memcpy(&(convetInfo.rAEInfo), &(m_rIspCamInfoP2.rAEInfo), sizeof(AE_INFO_T));
        convetInfo.eIdx_ISO_U = m_rIspCamInfoP2.eIdx_ISO_U;
        convetInfo.eIdx_ISO_L = m_rIspCamInfoP2.eIdx_ISO_L;
        m_bSetAeInfoP2 = MFALSE;
        memcpy((void*)(&m_rIspCamInfoP2), &convetInfo, sizeof(RAWIspCamInfo));
        return m_rIspCamInfoP2;
    } else {
        return m_rIspCamInfo;
    }
}

EIndex_ISO_T
Paramctrl::
map_ISO_value2index(MUINT32 const u4Iso) const
{
    MUINT32 t_low = 0, t_high = m_rIspParam.ISPRegs.Iso_Env.u2Length - 1;

    MUINT32 t_Iso = u4Iso;
    MUINT32 t_ceiling =m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[t_high];

    if( t_Iso >= t_ceiling){
           t_Iso = t_ceiling - 1;
    }


    while ( (t_low <= t_high) && (t_high > 1) )
    {
        int t_mid = (t_low + t_high)/2;
        MUINT32 t_u4mid = m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[t_mid];

        if ( (t_Iso < t_u4mid) && (t_Iso >= (MUINT32)m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[t_mid-1]) )
        {
            return static_cast<EIndex_ISO_T>(t_mid);
        }
        else if (t_Iso >= t_u4mid)
        {
            t_low = t_mid + 1 ;
        }
        else if (t_Iso < t_u4mid)
        {
            t_high = t_mid - 1 ;
        }
     }

    if (t_high < 2){
        if ( (t_Iso < (MUINT32)m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[t_high]) && (t_Iso >= (MUINT32)m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[t_low])){
            return static_cast<EIndex_ISO_T>(t_high);
        }
        else{
            return static_cast<EIndex_ISO_T>(t_low);
        }
    }

    CAM_LOGE("ISO_MAPPING_ERROR");
    return eIDX_ISO_0;  //default
}


EIndex_ZOOM_T
Paramctrl::
map_Zoom_value2index(MINT32 const i4ZoomRatio_x100) const
{
    MUINT16 length = m_rIspParam.ISPRegs.Zoom_Env.u2Length;

    for(int i=0; i <= length; i++){
        if(i4ZoomRatio_x100 < m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[i])
            return static_cast<EIndex_ZOOM_T>(i);
    }

    return static_cast<EIndex_ZOOM_T>(length-1);  //max
}

EIndex_ZOOM_T
Paramctrl::
zoom_index_transform(MINT32 const i4ZoomRatio_x100, EIndex_ZOOM_T const ZoomIdx) const
{
    if ((i4ZoomRatio_x100 > m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[eIDX_ZOOM_OFF])
        && (i4ZoomRatio_x100 < m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[eIDX_ZOOM_1]))
        return eIDX_ZOOM_0;
    else
        return ZoomIdx;
}

MUINT32
Paramctrl::
map_ISO_index2value(EIndex_ISO_T const eIdx) const
{
    EIndex_ISO_T tIdx = eIdx;

    if(tIdx>(m_rIspParam.ISPRegs.Iso_Env.u2Length-1))
        tIdx = (EIndex_ISO_T)(m_rIspParam.ISPRegs.Iso_Env.u2Length-1);

    return (MUINT32)m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[tIdx];
}


MVOID
Paramctrl::
get_ISOIdx_neighbor(RAWIspCamInfo& rIspCamInfo, EIndex_ISO_T const IsoIdx, MUINT32 const u4Iso)
{
      if (u4Iso >= (EIndex_ISO_T)m_rIspParam.ISPRegs.Iso_Env.IDX_Partition[m_rIspParam.ISPRegs.Iso_Env.u2Length-1]){
        rIspCamInfo.eIdx_ISO_L = static_cast<EIndex_ISO_T>(m_rIspParam.ISPRegs.Iso_Env.u2Length - 1);
        rIspCamInfo.eIdx_ISO_U = static_cast<EIndex_ISO_T>(m_rIspParam.ISPRegs.Iso_Env.u2Length - 1);
      }
      else if (IsoIdx == 0){
        rIspCamInfo.eIdx_ISO_L = static_cast<EIndex_ISO_T>(0);
        rIspCamInfo.eIdx_ISO_U = static_cast<EIndex_ISO_T>(0);
      }
      else{
        rIspCamInfo.eIdx_ISO_L = static_cast<EIndex_ISO_T>(IsoIdx - 1);
        rIspCamInfo.eIdx_ISO_U = IsoIdx;

      }
}

MVOID
Paramctrl::
get_ZoomIdx_neighbor(RAWIspCamInfo& rIspCamInfo, EIndex_ZOOM_T const ZoomIdx, MUINT32 const u4Zoom)
{
      if (u4Zoom >= (MUINT32)m_rIspParam.ISPRegs.Zoom_Env.IDX_Partition[m_rIspParam.ISPRegs.Zoom_Env.u2Length-1]){
        rIspCamInfo.eIdx_Zoom_L = static_cast<EIndex_ZOOM_T>(m_rIspParam.ISPRegs.Zoom_Env.u2Length - 1);
        rIspCamInfo.eIdx_Zoom_U = static_cast<EIndex_ZOOM_T>(m_rIspParam.ISPRegs.Zoom_Env.u2Length - 1);
      }
      else if (ZoomIdx == 0){
        rIspCamInfo.eIdx_Zoom_L = static_cast<EIndex_ZOOM_T>(0);
        rIspCamInfo.eIdx_Zoom_U = static_cast<EIndex_ZOOM_T>(0);
      }
      else{
        rIspCamInfo.eIdx_Zoom_L = static_cast<EIndex_ZOOM_T>(ZoomIdx - 1);
        rIspCamInfo.eIdx_Zoom_U = ZoomIdx;
      }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setISPAEGain(MBOOL bEnableWorkAround, MUINT32 u4NewIspAEGain)
{
    MUINT32 u4CurrentIspGain = 0;

    if(bEnableWorkAround == MTRUE) {   // enable iVHDR work around
        //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).getIspAEGain(&u4CurrentIspGain);
        if (checkParamChange(u4CurrentIspGain, u4NewIspAEGain)) {
            //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).setIspAEGain(u4NewIspAEGain);
            ISP_MGR_OBC_T::getInstance(m_eSensorDev).setIspAEGain(512);
        }
    } else {
        ISP_MGR_OBC_T::getInstance(m_eSensorDev).getIspAEGain(&u4CurrentIspGain);
        if (checkParamChange(u4CurrentIspGain, u4NewIspAEGain)) {
            //ISP_MGR_AWB_STAT_CONFIG_T::getInstance(m_eSensorDev).setIspAEGain(512);
            ISP_MGR_OBC_T::getInstance(m_eSensorDev).setIspAEGain(u4NewIspAEGain);
        }
    }
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setIspFlareGainOffset(MINT16 i2FlareGain, MINT16 i2FlareOffset)
{
    CAM_LOGD_IF(m_bDebugEnable, "setIspFlareGainOffset(gain, offset)=(%d, %d)", i2FlareGain, i2FlareOffset);

    if(m_pIspTuningCustom->is_to_invoke_flc(m_rIspCamInfo)){
        ISP_MGR_RPG_T::getInstance(m_eSensorDev).setIspFlare(FLARE_SCALE_UNIT, 0);
    }
    else{
        ISP_MGR_RPG_T::getInstance(m_eSensorDev).setIspFlare(i2FlareGain, i2FlareOffset);
    }
    //ISP_MGR_PGN_T::getInstance(m_eSensorDev).setIspFlare(i2FlareGain, i2FlareOffset);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setAFInfo(AF_INFO_T const &rAFInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "setAFInfo()");

    Mutex::Autolock lock(m_Lock);

    m_rIspCamInfo.rAFInfo = rAFInfo;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFlashInfo(FLASH_INFO_T const &rFlashInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "setFlashInfo()");

    Mutex::Autolock lock(m_Lock);

    m_rIspCamInfo.rFlashInfo = rFlashInfo;

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setIndex_Shading(MINT32 const i4IDX)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] idx %d", __FUNCTION__, i4IDX);

    Mutex::Autolock lock(m_Lock);

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

    Mutex::Autolock lock(m_Lock);

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
setTGInfo(MINT32 const i4TGInfo)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] i4TGInfo = %d", __FUNCTION__, i4TGInfo);

    Mutex::Autolock lock(m_Lock);

    switch (i4TGInfo)
    {
    case CAM_TG_1:
        m_eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        m_eSensorTG = ESensorTG_2;
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

    Mutex::Autolock lock(m_Lock);

    m_rIspCamInfo.fgFDEnable = FD_enable;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFDInfo(float const FD_ratio)
{
    MY_LOG_IF(m_bDebugEnable, "[%s] FD_ratio = %f", __FUNCTION__, FD_ratio);

    Mutex::Autolock lock(m_Lock);

    if(m_rIspCamInfo.fgFDEnable){
        m_rIspCamInfo.FDRatio = FD_ratio;
    }
    else{
        m_rIspCamInfo.FDRatio = 0;
    }

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setCamMode(MINT32 const i4CamMode)
{
    if (m_i4CamMode != i4CamMode) {
        m_i4CamMode = i4CamMode;
        CAM_LOGD_IF(m_bDebugEnable, "m_i4CamMode = %d", m_i4CamMode);

        switch (m_i4CamMode)
        {
        case eAppMode_EngMode:
             char fileName[] = "/sdcard/isp/isp_nvram_anr.table";
             char fileName2[] = "/sdcard/isp/isp_nvram_ccr.table";
             MINT32 ret = 0;
             ret = mkdir("/sdcard/isp", S_IRWXU | S_IRWXG | S_IRWXO);
             if (ret < 0)
             {
                 MY_LOGE("mkdir /sdcard/isp fail!");
             }
             FILE *fp = fopen(fileName, "w");
             if (!fp) {
                CAM_LOGE("fopen fail: /sdcard/isp/isp_nvram_anr.table");
             }
             else {
                 fwrite(reinterpret_cast<void *>(&m_rIspParam.ISPRegs.ANR[0]), 1, sizeof(ISP_NVRAM_ANR_T)*NVRAM_ANR_TBL_NUM, fp);
                 fclose(fp);
             }

             fp = fopen(fileName2, "w");
             if (!fp) {
                CAM_LOGE("fopen fail: /sdcard/isp/isp_nvram_ccr.table");
             }
             else {
                 fwrite(reinterpret_cast<void *>(&m_rIspParam.ISPRegs.CCR[0]), 1, sizeof(ISP_NVRAM_CCR_T)*NVRAM_CCR_TBL_NUM, fp);
                 fclose(fp);
             }
             break;
        }
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
        pIspGamma[i] = ggm.lut_g.lut[i].GGM_G;
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
MVOID
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
        break;
    case IspTuningMgr::E_ISPTUNING_NOTIFY_STOP:
        m_pGmaMgr->stop();
        m_pLceMgr->stop();
        break;
    case IspTuningMgr::E_ISPTUNING_SET_GMA_SCENARIO:
        m_pGmaMgr->setScenario(arg1);
        break;
    case IspTuningMgr::E_ISPTUNING_SET_GMA_AE_DYNAMIC:
        m_pGmaMgr->setAEStable((reinterpret_cast<IspTuningMgr::GMA_AE_DYNAMIC_INFO*>(arg1))->bStable);
        break;
    case IspTuningMgr::E_ISPTUNING_SET_LCE_SCENARIO:
        m_pLceMgr->setScenario(arg1);
        break;
    case IspTuningMgr::E_ISPTUNING_GET_ISP_GAMMA:
        getIspGamma(reinterpret_cast<NS3Av3::S3ACtrl_GetIspGamma*>(arg1));
        break;
    case IspTuningMgr::E_ISPTUNING_NOTIFY_SENSOR_TYPE:
        {
            m_u4RawFmtType = arg1;

            if(m_u4RawFmtType == SENSOR_RAW_MONO){
                ISP_MGR_CCM_T::getInstance(m_eSensorDev).setMono(MTRUE);
            }

            if(m_u4RawFmtType == SENSOR_RAW_RWB){
                ISP_MGR_CCM_T::getInstance(m_eSensorDev).setRWBSensorInfo(MTRUE);
            }

            if(m_u4RawFmtType == SENSOR_RAW_PD){
                ISP_MGR_BNR2_T::getInstance(m_eSensorDev).setPDSensorInfo((arg1 == SENSOR_RAW_PD ? MTRUE : MFALSE));

                CAMERA_BPCI_STRUCT* buf = NULL;
                int err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_PDC_TABLE, m_eSensorDev, (void*&)buf, 1);

                if (err != (int)NVRAM_DATA_READ_ERROR){
                    ISP_MGR_BNR2_T::getInstance(m_eSensorDev).setBPCIBuf(buf);
                }
                else{
                    CAM_LOGD("[%s] No BPCI table", __FUNCTION__);
                }
            }
        }
        break;
    case IspTuningMgr::E_ISPTUNING_GET_RWB_INFO:
        {
            //get gamma, degamma, linear GPU CCM
            //RWBTuning* pRWBTuning = reinterpret_cast<RWBTuning*>(arg1);
            //getGammaInfo(m_rIspCamInfo.rMtkGGM, pRWBTuning->GammaTableAddr, (MUINT32*)pRWBTuning->DeGammaTableAddr);
            //convertCcmFormat(m_pCcmMgr->getCCM(E_GPU_CCM), pRWBTuning->CCMTableAddr);
        }
        break;
    case IspTuningMgr::E_ISPTUNING_UPDATE_LSC_INFO:
        updateLscInfo(arg1);
        break;
    case IspTuningMgr::E_ISPTUNING_UPDATE_OBC_INFO:
        prepareHw_PerFrame_OBC();
        break;

    case IspTuningMgr::E_ISPTUNING_GET_LCE_GAIN:
        *(reinterpret_cast<MINT32*>(arg1)) = ISP_MGR_LCE_T::getInstance(m_eSensorDev).getLCE_Gain();
        break;

    default:
        CAM_LOGE("%s: ctrl not supported (ctrl, arg1, arg2) = (%d, %x, %x)", __FUNCTION__, ctrl, (MUINT32)arg1, (MUINT32)arg2);
        break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
setFlkEnable(MBOOL const flkEnable)
{
    CAM_LOGD_IF(m_bDebugEnable, "[%s] flkEnable = %d", __FUNCTION__, flkEnable);

    Mutex::Autolock lock(m_Lock);

    m_bFlkEnable = flkEnable;

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
Paramctrl::
setLCSOBuffer(void* lcsBuffer)
{
    m_pLCSBuffer = (IImageBuffer*) lcsBuffer;
}



