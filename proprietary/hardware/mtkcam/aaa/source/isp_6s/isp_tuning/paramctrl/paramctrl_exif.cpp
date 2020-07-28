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
#define LOG_TAG "paramctrl_exif"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <camera_feature.h>
//#include <awb_param.h>
//#include <ae_param.h>
//#include <af_param.h>
//#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include <ccm_mgr.h>
#include "paramctrl.h"
#include <isp_tuning_buf.h>

using namespace NSFeature;
using namespace NSIspTuning;
using namespace NSIspExifDebug;


#define _A_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

#define _A_P2(reg)\
    ((MUINT32)offsetof(dip_a_reg_t, reg))

namespace
{
//  Tag <- Isp Index.
inline
MVOID
setIspIdx(
    IspDebugTag (&rTags)[TagID_Total_Num],
    IspDebugTagID const eTagID,
    MINT32 const i4Idx
)
{
    rTags[eTagID].u4Val = i4Idx;
}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfoP1(const RAWIspCamInfo &rP1CamInfo, NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, MBOOL const /*fgReadFromHW*/) const
{
    std::lock_guard<std::mutex> lock(m_Lock);
#if 1
    ////////////////////////////////////////////////////////////////////////////
    //  (1) Header.
    ////////////////////////////////////////////////////////////////////////////
    rDebugInfo.hdr  = g_rIspExifDebugInfoHdr;

    ////////////////////////////////////////////////////////////////////////////
    //  (2) Body.
    ////////////////////////////////////////////////////////////////////////////
    IspDebugTag (&rTags)[TagID_Total_Num] = rDebugInfo.debugInfo.tags;

    // ISP debug tag version
    setIspIdx(rTags, IspTagVersion, IspDebugTagVersion);

    setIspIdx(rTags, M_P1_Profile,         rP1CamInfo.rMapping_Info.eIspProfile);
    setIspIdx(rTags, M_P1_SensorMode,      rP1CamInfo.rMapping_Info.eSensorMode);
    setIspIdx(rTags, M_P1_FrontalBin,      rP1CamInfo.rMapping_Info.eFrontBin);
    setIspIdx(rTags, M_P1_TargetSize,      rP1CamInfo.rMapping_Info.eSize);
    setIspIdx(rTags, M_P1_Flash,           rP1CamInfo.rMapping_Info.eFlash);
    setIspIdx(rTags, M_P1_APP,             rP1CamInfo.rMapping_Info.eApp);
    setIspIdx(rTags, M_P1_FaceDetection,   rP1CamInfo.rMapping_Info.eFaceDetection);
    setIspIdx(rTags, M_P1_DriverIC,        rP1CamInfo.rMapping_Info.eDriverIC);
    setIspIdx(rTags, M_P1_Custom_00,       rP1CamInfo.rMapping_Info.eCustom_00);
    setIspIdx(rTags, M_P1_Custom_01,       rP1CamInfo.rMapping_Info.eCustom_01);
    setIspIdx(rTags, M_P1_ZoomIdx,         rP1CamInfo.rMapping_Info.eZoom_Idx);
    setIspIdx(rTags, M_P1_LV_Idx,          rP1CamInfo.rMapping_Info.eLV_Idx);
    setIspIdx(rTags, M_P1_CT_Idx,          rP1CamInfo.rMapping_Info.eCT_Idx);
    setIspIdx(rTags, ATMS_SCENARIO_IDX_P1, m_rIdxCache.getSceneIdx(IDXCACHE_STAGE_P1));

    // RAWIspCamInfo
    setIspIdx(rTags, isHDR_P1,           rP1CamInfo.fgHDR);
    setIspIdx(rTags, UPPER_CT_IDX_P1,    rP1CamInfo.eIdx_CT_U);
    setIspIdx(rTags, LOWER_CT_IDX_P1,    rP1CamInfo.eIdx_CT_L);
    setIspIdx(rTags, UPPER_LV_IDX_P1,    rP1CamInfo.eIdx_LV_U);
    setIspIdx(rTags, LOWER_LV_IDX_P1,    rP1CamInfo.eIdx_LV_L);
    setIspIdx(rTags, ZoomRatio_x100_P1,  rP1CamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, ShadingIdx_P1,      rP1CamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, Custom_Interp_P1,   m_pIspTuningCustom->is_to_invoke_isp_interpolation(rP1CamInfo));

    //Adb control Interp
    setIspIdx(rTags, Adb_Interp, m_IspInterpCtrl);

    setIspIdx(rTags, Color_Method, m_rIspParam.ISPColorTbl.COMM.COLOR.COLOR_Method);

    //Operation mode
    setIspIdx(rTags, EOperMode, m_eOperMode);

    //Scene
    setIspIdx(rTags, SceneIdx_P1,        rP1CamInfo.eIdx_Scene);

    //Only P1 paramctrl info  RawAB TG Size
    setIspIdx(rTags, SensorRawFmt, m_u4RawFmtType);
    setIspIdx(rTags, SensorDev, m_eSensorDev);
    setIspIdx(rTags, SensorIdx, m_i4SensorIdx);
    //setIspIdx(rTags, SensorTG, rP1CamInfo.eSensorTG); //HHHHHH
    setIspIdx(rTags, P1_Path, m_ERawPath);

    //Size Info
    setIspIdx(rTags, TG_Size_W_P1YUV,       rP1CamInfo.rCropRzInfo.sTGout.w);
    setIspIdx(rTags, TG_Size_H_P1YUV,       rP1CamInfo.rCropRzInfo.sTGout.h);
    setIspIdx(rTags, FBinOnOff_P1YUV,       rP1CamInfo.rCropRzInfo.fgFBinOnOff);
    setIspIdx(rTags, RRZ_In_Size_W_P1YUV,   rP1CamInfo.rCropRzInfo.sRRZin.w);
    setIspIdx(rTags, RRZ_In_Size_H_P1YUV,   rP1CamInfo.rCropRzInfo.sRRZin.h);

    //AE Info
    setIspIdx(rTags, AE_INFO_EXP_TIME_P1, rP1CamInfo.rAEInfo.u8P1Exposuretime_ns/1000);
    setIspIdx(rTags, AE_INFO_SEN_GAIN_P1, rP1CamInfo.rAEInfo.u4P1SensorGain);
    setIspIdx(rTags, AE_INFO_ISP_GAIN_P1, rP1CamInfo.rAEInfo.u4P1DGNGain);
    setIspIdx(rTags, AE_INFO_REAL_ISO_P1, rP1CamInfo.rAEInfo.u4P1RealISOValue);
    setIspIdx(rTags, AE_INFO_GAIN_x100_P1, rP1CamInfo.rAEInfo.u4P1Gain_x100);

    //AWB Info
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_R_P1, rP1CamInfo.rAWBInfo.rCurrentAWBGain.i4R);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_G_P1, rP1CamInfo.rAWBInfo.rCurrentAWBGain.i4G);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_B_P1, rP1CamInfo.rAWBInfo.rCurrentAWBGain.i4B);
    setIspIdx(rTags, AWB_INFO_CT_P1,             rP1CamInfo.rAWBInfo.i4CCT);
    setIspIdx(rTags, AWB_INFO_WB_GAIN_R_P1,      rP1CamInfo.rAWBInfo.rRPG.i4R);
    setIspIdx(rTags, AWB_INFO_WB_GAIN_G_P1,      rP1CamInfo.rAWBInfo.rRPG.i4G);
    setIspIdx(rTags, AWB_INFO_WB_GAIN_B_P1,      rP1CamInfo.rAWBInfo.rRPG.i4B);

    ////////////////////////////////////////////////////////////////////////////
    // OBC
    setIspIdx(rTags, IDX_OBC_LOWER_P1, m_rIdxCache.getCacheValue(EModule_OBC, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_OBC_UPPER_P1, m_rIdxCache.getCacheValue(EModule_OBC, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_UPPERISO));
    // BPC
    setIspIdx(rTags, IDX_BPC_LOWER_P1, m_rIdxCache.getCacheValue(EModule_BPC_BPC, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_BPC_UPPER_P1, m_rIdxCache.getCacheValue(EModule_BPC_BPC, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_UPPERISO));
    // CT
    setIspIdx(rTags, IDX_CT_LOWER_P1, m_rIdxCache.getCacheValue(EModule_BPC_CT, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_CT_UPPER_P1, m_rIdxCache.getCacheValue(EModule_BPC_CT, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_UPPERISO));
    // PDC
    setIspIdx(rTags, IDX_PDC_LOWER_P1, m_rIdxCache.getCacheValue(EModule_BPC_PDC, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_PDC_UPPER_P1, m_rIdxCache.getCacheValue(EModule_BPC_PDC, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_UPPERISO));
    // DM
    setIspIdx(rTags, IDX_DM_LOWER_P1, m_rIdxCache.getCacheValue(EModule_DM, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_DM_UPPER_P1, m_rIdxCache.getCacheValue(EModule_DM, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_UPPERISO));
    // GGM
    setIspIdx(rTags, IDX_GMA_P1, m_rIdxCache.getCacheValue(EModule_GMA, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_CURRENT));

    MUINT16 CCMidxU = 0;
    MUINT16 CCMidxL = 0;
    CCMidxU = m_rIdxCache.getCacheValue(EModule_CCM, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_UPPERLV);
    CCMidxL = m_rIdxCache.getCacheValue(EModule_CCM, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_LOWERLV);
    setIspIdx(rTags, SMOOTH_CCM_Ver             , m_ISP_ALG_VER.sCCM);
    setIspIdx(rTags, IDX_CCM_UPPER_LV_P1        , CCMidxU);
    setIspIdx(rTags, IDX_CCM_LOWER_LV_P1        , CCMidxL);
    setIspIdx(rTags, SMOOTH_CCM_CT_valid_NUM_UPPER_P1 , m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_CT_valid_NUM);
    setIspIdx(rTags, SMOOTH_CCM_CT_valid_NUM_LOWER_P1 , m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_CT_valid_NUM);
    setIspIdx(rTags, SMOOTH_CCM_Coef_UPPER_0_P1, m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_Coef[0]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_UPPER_1_P1, m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_Coef[1]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_UPPER_2_P1, m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_Coef[2]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_LOWER_0_P1, m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_Coef[0]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_LOWER_1_P1, m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_Coef[1]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_LOWER_2_P1, m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_Coef[2]);

    setIspIdx(rTags, IDX_LTM, m_rIdxCache.getCacheValue(EModule_LTM, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_CURRENT));
    setIspIdx(rTags, IDX_HLR, m_rIdxCache.getCacheValue(EModule_HLR, IDXCACHE_STAGE_P1, IDXCACHE_VALTYPE_CURRENT));

    MUINT32 u4RegCnt_start = 0;
    MUINT32 u4RegCnt = 0;
    getP1ExifSizeInfo(u4RegCnt, u4RegCnt_start);

    MVOID* pP1TuningBuffer = IspTuningBufCtrl::getInstance(m_eSensorDev)->getP1Buffer();

    if(u4RegCnt > P1_BUFFER_SIZE)
    {
        CAM_LOGE("P1 buffer:(%d) byte exceed EXIF allocation size:(%d) byte", u4RegCnt*4, P1_BUFFER_SIZE*4);
    }
    else
    {
        if (u4RegCnt_start != P1_START_ADDR_COUNT)
                CAM_LOGW("[%s()] u4RegCnt_start(%d), P1_START_ADDR_COUNT(%d)",
                __FUNCTION__,u4RegCnt_start, P1_START_ADDR_COUNT);

        if (pP1TuningBuffer != NULL)
        {
            memcpy(rDebugInfo.P1RegInfo.regData, (MINT32 *)pP1TuningBuffer+u4RegCnt_start, P1_BUFFER_SIZE*4);
            rDebugInfo.P1RegInfo.u4TableSize = P1_BUFFER_SIZE+1;
            rDebugInfo.P1RegInfo.u4HwVersion = 9;
        }
    }
#endif
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfoP2(const NSIspTuning::ISP_INFO_T& rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, const void* pRegBuf, const void* pMfbBuf ) const
{
    std::lock_guard<std::mutex> lock(m_Lock);
#if 1
    ////////////////////////////////////////////////////////////////////////////
    //  (1) Header.
    ////////////////////////////////////////////////////////////////////////////
    rDebugInfo.hdr  = g_rIspExifDebugInfoHdr;

    ////////////////////////////////////////////////////////////////////////////
    //  (2) Body.
    ////////////////////////////////////////////////////////////////////////////
    IspDebugTag (&rTags)[TagID_Total_Num] = rDebugInfo.debugInfo.tags;
    const NSIspTuning::RAWIspCamInfo& rCamInfo = rIspInfo.rCamInfo;

    // ISP debug tag version
    setIspIdx(rTags, IspTagVersion, IspDebugTagVersion);
    setIspIdx(rTags, UniqueKey, rIspInfo.i4UniqueKey);
    ////////////////////////////////////////////////////////////////////////////
    setIspIdx(rTags, M_P2_Profile,       rCamInfo.rMapping_Info.eIspProfile);
    setIspIdx(rTags, M_P2_SensorMode,    rCamInfo.rMapping_Info.eSensorMode);
    setIspIdx(rTags, M_P2_FrontalBin,    rCamInfo.rMapping_Info.eFrontBin);
    setIspIdx(rTags, M_P2_TargetSize,    rCamInfo.rMapping_Info.eSize);
    setIspIdx(rTags, M_P2_Flash,         rCamInfo.rMapping_Info.eFlash);
    setIspIdx(rTags, M_P2_APP,           rCamInfo.rMapping_Info.eApp);
    setIspIdx(rTags, M_P2_FaceDetection, rCamInfo.rMapping_Info.eFaceDetection);
    setIspIdx(rTags, M_P2_DriverIC,      rCamInfo.rMapping_Info.eDriverIC);
    setIspIdx(rTags, M_P2_Custom_00,     rCamInfo.rMapping_Info.eCustom_00);
    setIspIdx(rTags, M_P2_Custom_01,     rCamInfo.rMapping_Info.eCustom_01);
    setIspIdx(rTags, M_P2_ZoomIdx,       rCamInfo.rMapping_Info.eZoom_Idx);
    setIspIdx(rTags, M_P2_LV_Idx,        rCamInfo.rMapping_Info.eLV_Idx);
    setIspIdx(rTags, M_P2_CT_Idx,        rCamInfo.rMapping_Info.eCT_Idx);
    setIspIdx(rTags, ATMS_SCENARIO_IDX_P2,       m_rIdxCache.getSceneIdx(IDXCACHE_STAGE_P2));

    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_EXIF,"RAWIspCamInfo:");
    setIspIdx(rTags, isHDR_P2,      rCamInfo.fgHDR);
    setIspIdx(rTags, UPPER_CT_IDX_P2,    rCamInfo.eIdx_CT_U);
    setIspIdx(rTags, LOWER_CT_IDX_P2,    rCamInfo.eIdx_CT_L);
    setIspIdx(rTags, UPPER_LV_IDX_P2,    rCamInfo.eIdx_LV_U);
    setIspIdx(rTags, LOWER_LV_IDX_P2,    rCamInfo.eIdx_LV_L);
    setIspIdx(rTags, ZoomRatio_x100_P2, rCamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, ShadingIdx_P2, rCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, Custom_Interp_P2, m_pIspTuningCustom->is_to_invoke_isp_interpolation(rCamInfo));

    //Scene
    setIspIdx(rTags, SceneIdx_P2, rCamInfo.eIdx_Scene);
    //EffectMode
    setIspIdx(rTags, EffectMode_P2, rCamInfo.eIdx_Effect);
    //UserSelectLevel
    setIspIdx(rTags, EdgeIdx_P2, rCamInfo.rIspUsrSelectLevel.eIdx_Edge);
    setIspIdx(rTags, HueIdx_P2, rCamInfo.rIspUsrSelectLevel.eIdx_Hue);
    setIspIdx(rTags, SatIdx_P2, rCamInfo.rIspUsrSelectLevel.eIdx_Sat);
    setIspIdx(rTags, BrightIdx_P2, rCamInfo.rIspUsrSelectLevel.eIdx_Bright);
    setIspIdx(rTags, ContrastIdx_P2, rCamInfo.rIspUsrSelectLevel.eIdx_Contrast);

    //LCS Info
    setIspIdx(rTags, FrontBin_ByLCS, rCamInfo.rLCS_Info.bFrontBin);
    setIspIdx(rTags, QBin_Rto_ByLCS, rCamInfo.rLCS_Info.u4Qbin_Rto);

    //Size Info
    setIspIdx(rTags, TG_Size_W,       rCamInfo.rCropRzInfo.sTGout.w);
    setIspIdx(rTags, TG_Size_H,       rCamInfo.rCropRzInfo.sTGout.h);
    setIspIdx(rTags, FBinOnOff,       rCamInfo.rCropRzInfo.fgFBinOnOff);
    setIspIdx(rTags, RRZOnOff,        rCamInfo.rCropRzInfo.fgRRZOnOff);
    setIspIdx(rTags, RRZ_In_Size_W,   rCamInfo.rCropRzInfo.sRRZin.w);
    setIspIdx(rTags, RRZ_In_Size_H,   rCamInfo.rCropRzInfo.sRRZin.h);
    setIspIdx(rTags, RRZ_Crop_Ofst_X, rCamInfo.rCropRzInfo.rRRZcrop.p.x);
    setIspIdx(rTags, RRZ_Crop_Ofst_Y, rCamInfo.rCropRzInfo.rRRZcrop.p.y);
    setIspIdx(rTags, RRZ_Crop_Size_W, rCamInfo.rCropRzInfo.rRRZcrop.s.w);
    setIspIdx(rTags, RRZ_Crop_Size_H, rCamInfo.rCropRzInfo.rRRZcrop.s.h);
    setIspIdx(rTags, RRZ_Out_W,       rCamInfo.rCropRzInfo.sRRZout.w);
    setIspIdx(rTags, RRZ_Out_H,       rCamInfo.rCropRzInfo.sRRZout.h);

    //Metadata
    setIspIdx(rTags, Meta_PureRaw,    (rCamInfo.i4RawType == NSIspTuning::ERawType_Pure) ? 1 : 0);
    setIspIdx(rTags, Meta_PGNEnable,   !(rCamInfo.fgRPGEnable));
    setIspIdx(rTags, Meta_ByPassLCE,   rCamInfo.bBypassLCE);
    setIspIdx(rTags, Meta_ByPassNR,    rCamInfo.bBypassNR);
    setIspIdx(rTags, Meta_UpdateMode,  rCamInfo.u1P2TuningUpdate);
    setIspIdx(rTags, Meta_P2InFmt,     rCamInfo.i4P2InImgFmt);
    // LCEBuffer
    setIspIdx(rTags, BPCBuffer,       (ISP_MGR_BPC::getInstance(m_eSensorDev).getDMGItable() ? 1 : 0));
    setIspIdx(rTags, LCEBuffer,       (m_pLCESBuffer) ? 1 : 0);
    setIspIdx(rTags, LCESHOBuffer,    (m_pLCESHOBuffer) ? 1 : 0);
    setIspIdx(rTags, DCEBuffer,       (m_pDCESBuffer) ? 1 : 0);

    //AE Info
    setIspIdx(rTags, AE_INFO_EXP_TIME_P2, rCamInfo.rAEInfo.u8P2Exposuretime_ns/1000);
    setIspIdx(rTags, AE_INFO_AFE_GAIN_P2, rCamInfo.rAEInfo.u4P2SensorGain);
    setIspIdx(rTags, AE_INFO_ISP_GAIN_P2, rCamInfo.rAEInfo.u4P2DGNGain);
    setIspIdx(rTags, AE_INFO_REAL_ISO_P2, rCamInfo.rAEInfo.u4P2RealISOValue);
    setIspIdx(rTags, AE_INFO_GAIN_x100_P2,rCamInfo.rAEInfo.u4P2Gain_x100);
    setIspIdx(rTags, AE_INFO_LV_x10,      rCamInfo.rAEInfo.i4RealLightValue_x10);

    //AWB Info
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_R_P2, rCamInfo.rAWBInfo.rCurrentAWBGain.i4R);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_G_P2, rCamInfo.rAWBInfo.rCurrentAWBGain.i4G);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_B_P2, rCamInfo.rAWBInfo.rCurrentAWBGain.i4B);
    setIspIdx(rTags, AWB_INFO_CT_P2,             rCamInfo.rAWBInfo.i4CCT);
    setIspIdx(rTags, AWB_INFO_WB_GAIN_R_P2,      rCamInfo.rAWBInfo.rPGN.i4R);
    setIspIdx(rTags, AWB_INFO_WB_GAIN_G_P2,      rCamInfo.rAWBInfo.rPGN.i4G);
    setIspIdx(rTags, AWB_INFO_WB_GAIN_B_P2,      rCamInfo.rAWBInfo.rPGN.i4B);

    setIspIdx(rTags, HighQCap_P2, rCamInfo.fgNeedKeepP1);

    // OBC
    setIspIdx(rTags, IDX_OBC_LOWER_P2, m_rIdxCache.getCacheValue(EModule_OBC, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_OBC_UPPER_P2, m_rIdxCache.getCacheValue(EModule_OBC, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // BPC
    setIspIdx(rTags, IDX_BPC_LOWER_P2, m_rIdxCache.getCacheValue(EModule_BPC_BPC, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_BPC_UPPER_P2, m_rIdxCache.getCacheValue(EModule_BPC_BPC, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // CT
    setIspIdx(rTags, IDX_CT_LOWER_P2, m_rIdxCache.getCacheValue(EModule_BPC_CT, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_CT_UPPER_P2, m_rIdxCache.getCacheValue(EModule_BPC_CT, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // PDC
    setIspIdx(rTags, IDX_PDC_LOWER_P2, m_rIdxCache.getCacheValue(EModule_BPC_PDC, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_PDC_UPPER_P2, m_rIdxCache.getCacheValue(EModule_BPC_PDC, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // DM
    setIspIdx(rTags, IDX_DM_LOWER_P2, m_rIdxCache.getCacheValue(EModule_DM, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_DM_UPPER_P2, m_rIdxCache.getCacheValue(EModule_DM, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // YNR
    setIspIdx(rTags, IDX_YNR_LOWER_P2, m_rIdxCache.getCacheValue(EModule_YNR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_YNR_UPPER_P2, m_rIdxCache.getCacheValue(EModule_YNR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // YNR_TBL
    setIspIdx(rTags, IDX_YNR_TBL_LOWER_P2, m_rIdxCache.getCacheValue(EModule_YNR_TBL, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_YNR_TBL_UPPER_P2, m_rIdxCache.getCacheValue(EModule_YNR_TBL, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // CNR
    setIspIdx(rTags, IDX_CNR_LOWER_P2, m_rIdxCache.getCacheValue(EModule_CNR_CNR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_CNR_UPPER_P2, m_rIdxCache.getCacheValue(EModule_CNR_CNR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // CCR
    setIspIdx(rTags, IDX_CCR_LOWER_P2, m_rIdxCache.getCacheValue(EModule_CNR_CCR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_CCR_UPPER_P2, m_rIdxCache.getCacheValue(EModule_CNR_CCR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    // ABF
    setIspIdx(rTags, IDX_ABF_LOWER_P2, m_rIdxCache.getCacheValue(EModule_CNR_ABF, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_ABF_UPPER_P2, m_rIdxCache.getCacheValue(EModule_CNR_ABF, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    //EE
    setIspIdx(rTags, IDX_EE_LOWER_P2, m_rIdxCache.getCacheValue(EModule_EE, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_EE_UPPER_P2, m_rIdxCache.getCacheValue(EModule_EE, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    //NR3D
    setIspIdx(rTags, IDX_NR3D_LOWER_P2, m_rIdxCache.getCacheValue(EModule_NR3D, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_NR3D_UPPER_P2, m_rIdxCache.getCacheValue(EModule_NR3D, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    //MFB
    setIspIdx(rTags, IDX_MFB_LOWER_P2, m_rIdxCache.getCacheValue(EModule_MFB, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_MFB_UPPER_P2, m_rIdxCache.getCacheValue(EModule_MFB, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    //MIX
    setIspIdx(rTags, IDX_MIX_LOWER_P2, m_rIdxCache.getCacheValue(EModule_MIX, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_MIX_UPPER_P2, m_rIdxCache.getCacheValue(EModule_MIX, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));

    setIspIdx(rTags, SMOOTH_COLOR_Ver, m_ISP_ALG_VER.sCOLOR);
    setIspIdx(rTags, IDX_COLOR_UPPER_LV_UPPER_CT, m_rIdxCache.getCacheValue(EModule_COLOR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERLV_UPPERCT));
    setIspIdx(rTags, IDX_COLOR_LOWER_LV_UPPER_CT, m_rIdxCache.getCacheValue(EModule_COLOR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERLV_UPPERCT));
    setIspIdx(rTags, IDX_COLOR_UPPER_LV_LOWER_CT, m_rIdxCache.getCacheValue(EModule_COLOR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERLV_LOWERCT));
    setIspIdx(rTags, IDX_COLOR_LOWER_LV_LOWER_CT, m_rIdxCache.getCacheValue(EModule_COLOR, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERLV_LOWERCT));
    setIspIdx(rTags, IDX_COLOR_PARAM_IDX        , m_rIdxCache.getCacheValue(EModule_COLOR_PARAM, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_CURRENT));

    //CCM
    MUINT16 CCMidxU = 0;
    MUINT16 CCMidxL = 0;
    CCMidxU = m_rIdxCache.getCacheValue(EModule_CCM, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERLV);
    CCMidxL = m_rIdxCache.getCacheValue(EModule_CCM, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERLV);
    setIspIdx(rTags, IDX_CCM_LOWER_LV_P2        , CCMidxL);
    setIspIdx(rTags, IDX_CCM_UPPER_LV_P2        , CCMidxU);
    setIspIdx(rTags, SMOOTH_CCM_CT_valid_NUM_UPPER_P2 , m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_CT_valid_NUM);
    setIspIdx(rTags, SMOOTH_CCM_CT_valid_NUM_LOWER_P2 , m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_CT_valid_NUM);
    setIspIdx(rTags, SMOOTH_CCM_Coef_UPPER_0_P2, m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_Coef[0]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_UPPER_1_P2, m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_Coef[1]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_UPPER_2_P2, m_rIspParam.ISPColorTbl.CCM[CCMidxU].CCM_Coef[2]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_LOWER_0_P2, m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_Coef[0]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_LOWER_1_P2, m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_Coef[1]);
    setIspIdx(rTags, SMOOTH_CCM_Coef_LOWER_2_P2, m_rIspParam.ISPColorTbl.CCM[CCMidxL].CCM_Coef[2]);

    setIspIdx(rTags, IDX_FW_YNR_FD_LOWER, m_rIdxCache.getCacheValue(EModule_YNR_FD, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, IDX_FW_YNR_FD_UPPER, m_rIdxCache.getCacheValue(EModule_YNR_FD, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    setIspIdx(rTags, FW_YNR_FD_MAP_WIDTH, m_FaceIsize.w );
    setIspIdx(rTags, FW_YNR_FD_MAP_HEIGHT, m_FaceIsize.h);
    setIspIdx(rTags, FW_YNR_FD_TBL_EN,            m_ynr_fd.TBL.YNR_FD_TBL_EN);
    setIspIdx(rTags, FW_YNR_FD_TBL_M_GAIN,        m_ynr_fd.TBL.YNR_FD_TBL_M_GAIN);
    setIspIdx(rTags, FW_YNR_FD_TBL_F_GAIN,        m_ynr_fd.TBL.YNR_FD_TBL_F_GAIN);
    setIspIdx(rTags, FW_YNR_FD_TBL_Y_GAIN,        m_ynr_fd.TBL.YNR_FD_TBL_Y_GAIN);
    setIspIdx(rTags, FW_YNR_FD_TBL_U_GAIN,        m_ynr_fd.TBL.YNR_FD_TBL_U_GAIN);
    setIspIdx(rTags, FW_YNR_FD_TBL_V_GAIN,        m_ynr_fd.TBL.YNR_FD_TBL_V_GAIN);
    setIspIdx(rTags, FW_YNR_FD_TBL_Y_Range,       m_ynr_fd.TBL.YNR_FD_TBL_Y_Range);
    setIspIdx(rTags, FW_YNR_FD_TBL_U_Range,       m_ynr_fd.TBL.YNR_FD_TBL_U_Range);
    setIspIdx(rTags, FW_YNR_FD_TBL_V_Range,       m_ynr_fd.TBL.YNR_FD_TBL_V_Range);
    setIspIdx(rTags, FW_YNR_FD_MAP_EN,            m_ynr_fd.MAP1.YNR_FD_MAP_EN);
    setIspIdx(rTags, FW_YNR_FD_MAP_MAX_GAIN,      m_ynr_fd.MAP1.YNR_FD_MAP_MAX_GAIN);
    setIspIdx(rTags, FW_YNR_FD_MAP_M_GAIN,        m_ynr_fd.MAP1.YNR_FD_MAP_M_GAIN);
    setIspIdx(rTags, FW_YNR_FD_MAP_F_GAIN,        m_ynr_fd.MAP1.YNR_FD_MAP_F_GAIN);
    setIspIdx(rTags, FW_YNR_FD_MAP_W_R,           m_ynr_fd.MAP1.YNR_FD_MAP_W_R);
    setIspIdx(rTags, FW_YNR_FD_MAP_H_R,           m_ynr_fd.MAP1.YNR_FD_MAP_H_R);
    setIspIdx(rTags, FW_YNR_FD_MAP_DOWN,          m_ynr_fd.MAP1.YNR_FD_MAP_DOWN);
    setIspIdx(rTags, FW_YNR_FD_MAP_Y_RANGE,       m_ynr_fd.MAP1.YNR_FD_MAP_Y_RANGE);
    setIspIdx(rTags, FW_YNR_FD_MAP_U_RANGE,       m_ynr_fd.MAP1.YNR_FD_MAP_U_RANGE);
    setIspIdx(rTags, FW_YNR_FD_MAP_V_RANGE,       m_ynr_fd.MAP1.YNR_FD_MAP_V_RANGE);
    setIspIdx(rTags, FW_YNR_FD_MAP_EYES_EN,       m_ynr_fd.MAP2.rsv1);
    setIspIdx(rTags, FW_YNR_FD_MAP_EYES_SIZE_R,   m_ynr_fd.MAP2.rsv2);
    setIspIdx(rTags, FW_YNR_FD_MAP_EYES_CV_TH0,   m_ynr_fd.MAP2.rsv3);
    setIspIdx(rTags, FW_YNR_FD_MAP_EYES_CV_TH1,   m_ynr_fd.MAP2.rsv4);
    setIspIdx(rTags, FW_YNR_FD_MAP_SIZE_TH0,      m_ynr_fd.MAP2.rsv5);
    setIspIdx(rTags, FW_YNR_FD_MAP_SIZE_TH1,      m_ynr_fd.MAP2.rsv6);
    setIspIdx(rTags, FW_YNR_FD_MAP_SIZE_GAIN1,    m_ynr_fd.MAP2.rsv7);
    setIspIdx(rTags, FW_YNR_FD_MAP_GENDER_BAL_EN, m_ynr_fd.MAP2.rsv8);
    setIspIdx(rTags, FW_YNR_FD_MAP_U_GAIN,        m_ynr_fd.MAP2.rsv9);
    setIspIdx(rTags, FW_YNR_FD_RSV10,   m_ynr_fd.MAP2.rsv10);
    setIspIdx(rTags, FW_YNR_FD_RSV11,   m_ynr_fd.MAP2.rsv11);
    setIspIdx(rTags, FW_YNR_FD_RSV12,   m_ynr_fd.MAP2.rsv12);
    setIspIdx(rTags, FW_YNR_FD_RSV13,   m_ynr_fd.MAP2.rsv13);
    setIspIdx(rTags, FW_YNR_FD_RSV14,   m_ynr_fd.MAP2.rsv14);
    setIspIdx(rTags, FW_YNR_FD_RSV15,   m_ynr_fd.MAP2.rsv15);
    setIspIdx(rTags, FW_YNR_FD_RSV16,   m_ynr_fd.MAP2.rsv16);
    setIspIdx(rTags, FW_YNR_FD_RSV17,   m_ynr_fd.MAP2.rsv17);
    setIspIdx(rTags, FW_YNR_FD_RSV18,   m_ynr_fd.MAP2.rsv18);
    setIspIdx(rTags, FW_YNR_FD_RSV19,   m_ynr_fd.MAP2.rsv19);
    setIspIdx(rTags, FW_YNR_FD_RSV20,   m_ynr_fd.MAP2.rsv20);
    setIspIdx(rTags, FW_YNR_FD_RSV21,   m_ynr_fd.MAP2.rsv21);
    setIspIdx(rTags, FW_YNR_FD_RSV22,   m_ynr_fd.MAP2.rsv22);
    setIspIdx(rTags, FW_YNR_FD_RSV23,   m_ynr_fd.MAP2.rsv23);
    setIspIdx(rTags, FW_YNR_FD_RSV24,   m_ynr_fd.MAP2.rsv24);
    setIspIdx(rTags, FW_YNR_FD_RSV25,   m_ynr_fd.MAP2.rsv25);
    setIspIdx(rTags, FW_YNR_FD_RSV26,   m_ynr_fd.MAP2.rsv26);

    setIspIdx(rTags, IDX_FW_YNR_LCE_UPPER, m_rIdxCache.getCacheValue(EModule_YNR_LCE_LINK, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_UPPERISO));
    setIspIdx(rTags, IDX_FW_YNR_LCE_LOWER, m_rIdxCache.getCacheValue(EModule_YNR_LCE_LINK, IDXCACHE_STAGE_P2, IDXCACHE_VALTYPE_LOWERISO));
    setIspIdx(rTags, FW_YNR_VER, m_SW_YNR_LTM_Ver);
    setIspIdx(rTags, FW_YNR_TC_LINK_STR, m_sw_ynr.LINK1.FW_YNR_TC_LINK_STR);
    setIspIdx(rTags, FW_YNR_TC_LUMA_LINK_STR, m_sw_ynr.LINK1.FW_YNR_TC_LINK_STR_LUMA);
    setIspIdx(rTags, FW_YNR_CE_LINK_STR, m_sw_ynr.LINK1.FW_YNR_CE_LINK_STR);
    setIspIdx(rTags, FW_YNR_GAIN_CLIP_HI, m_sw_ynr.LINK1.FW_YNR_GAIN_CLIP_HI);
    setIspIdx(rTags, FW_YNR_GAIN_CLIP_LO, m_sw_ynr.LINK1.FW_YNR_GAIN_CLIP_LO);
    setIspIdx(rTags, FW_YNR_GAMMA_SMOOTH, m_sw_ynr.LINK1.FW_YNR_GAMMA_SMOOTH);
    setIspIdx(rTags, FW_YNR_P1_GAIN, m_sw_ynr.LINK1.FW_YNR_P1_GAIN);
    setIspIdx(rTags, FW_YNR_P50_GAIN, m_sw_ynr.LINK1.FW_YNR_P50_GAIN);
    setIspIdx(rTags, FW_YNR_P250_GAIN, m_sw_ynr.LINK1.FW_YNR_P250_GAIN);
    setIspIdx(rTags, FW_YNR_P500_GAIN, m_sw_ynr.LINK1.FW_YNR_P500_GAIN);
    setIspIdx(rTags, FW_YNR_P750_GAIN, m_sw_ynr.LINK1.FW_YNR_P750_GAIN);
    setIspIdx(rTags, FW_YNR_P950_GAIN, m_sw_ynr.LINK1.FW_YNR_P950_GAIN);
    setIspIdx(rTags, FW_YNR_DCE_LINK_STR, m_sw_ynr.LINK2.FW_YNR_DCE_LINK_STR);
    setIspIdx(rTags, FW_YNR_DCE_GAIN_CLIP_HI, m_sw_ynr.LINK2.FW_YNR_DCE_GAIN_CLIP_HI);
    setIspIdx(rTags, FW_YNR_DCE_GAIN_CLIP_LO, m_sw_ynr.LINK2.FW_YNR_DCE_GAIN_CLIP_LO);
    setIspIdx(rTags, FW_YNR_LINK_BASE_GAIN, m_sw_ynr.LINK2.FW_YNR_LINK_BASE_GAIN);
    setIspIdx(rTags, FW_YNR_RSV1, m_sw_ynr.LINK2.rsv1);
    setIspIdx(rTags, FW_YNR_RSV2,m_sw_ynr.LINK2.rsv2);

    setIspIdx(rTags, ISO_THRESHOLD_ENABLE, m_bUseIsoThreshEnable);
    setIspIdx(rTags, ISO_THRESHOLD, m_isoThresh);
    setIspIdx(rTags, ISO_THRESHOLD_PREVIOUS_ISO, m_i4PreRealISO);

    //dynamic Gamma
    // adaptive Gamma
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_EXIF,"adaptive Gamma:");
    setIspIdx(rTags, IDX_GMA_P2, (MUINT32)m_pGmaMgr->getIDX());
    setIspIdx(rTags, SW_GMA_GMAMode, m_GmaExifInfo.i4GMAMode);
    setIspIdx(rTags, SW_GMA_FaceGMAFlag, m_GmaExifInfo.i4FaceGMAFlag);
    setIspIdx(rTags, SW_GMA_ChipVersion, m_GmaExifInfo.i4ChipVersion);
    setIspIdx(rTags, SW_GMA_MainVersion, m_GmaExifInfo.i4MainVersion);
    setIspIdx(rTags, SW_GMA_SubVersion, m_GmaExifInfo.i4SubVersion);
    setIspIdx(rTags, SW_GMA_SystemVersion, m_GmaExifInfo.i4SystemVersion);
    setIspIdx(rTags, SW_GMA_EVRatio, m_GmaExifInfo.u4EVRatio);
    setIspIdx(rTags, SW_GMA_LowContrastThr, m_GmaExifInfo.i4LowContrastThr);
    setIspIdx(rTags, SW_GMA_LowContrastRatio, m_GmaExifInfo.i4LowContrastRatio);
    setIspIdx(rTags, SW_GMA_LowContrastSeg, m_GmaExifInfo.i4LowContrastSeg);
    setIspIdx(rTags, SW_GMA_Contrast, m_GmaExifInfo.i4Contrast);
    setIspIdx(rTags, SW_GMA_Contrast_L, m_GmaExifInfo.i4Contrast_L);
    setIspIdx(rTags, SW_GMA_Contrast_H, m_GmaExifInfo.i4Contrast_H);
    setIspIdx(rTags, SW_GMA_HdrContrastWeight, m_GmaExifInfo.i4HdrContrastWeight);
    setIspIdx(rTags, SW_GMA_EVContrastY, m_GmaExifInfo.i4EVContrastY);
    setIspIdx(rTags, SW_GMA_ContrastY_L, m_GmaExifInfo.i4ContrastY_L);
    setIspIdx(rTags, SW_GMA_ContrastY_H, m_GmaExifInfo.i4ContrastY_H);
    setIspIdx(rTags, SW_GMA_NightContrastWeight, m_GmaExifInfo.i4NightContrastWeight);
    setIspIdx(rTags, SW_GMA_LV, m_GmaExifInfo.i4LV);
    setIspIdx(rTags, SW_GMA_LV_L, m_GmaExifInfo.i4LV_L);
    setIspIdx(rTags, SW_GMA_LV_H, m_GmaExifInfo.i4LV_H);
    setIspIdx(rTags, SW_GMA_HdrLVWeight, m_GmaExifInfo.i4HdrLVWeight);
    setIspIdx(rTags, SW_GMA_NightLVWeight, m_GmaExifInfo.i4NightLVWeight);
    setIspIdx(rTags, SW_GMA_SmoothEnable, m_GmaExifInfo.i4SmoothEnable);
    setIspIdx(rTags, SW_GMA_SmoothSpeed, m_GmaExifInfo.i4SmoothSpeed);
    setIspIdx(rTags, SW_GMA_SmoothWaitAE, m_GmaExifInfo.i4SmoothWaitAE);
    setIspIdx(rTags, SW_GMA_GMACurveEnable, m_GmaExifInfo.i4GMACurveEnable);
    setIspIdx(rTags, SW_GMA_CenterPt, m_GmaExifInfo.i4CenterPt);
    setIspIdx(rTags, SW_GMA_LowCurve, m_GmaExifInfo.i4LowCurve);
    setIspIdx(rTags, SW_GMA_SlopeL, m_GmaExifInfo.i4SlopeL);
    setIspIdx(rTags, SW_GMA_FlareEnable, m_GmaExifInfo.i4FlareEnable);
    setIspIdx(rTags, SW_GMA_FlareOffset, m_GmaExifInfo.i4FlareOffset);

    // adaptive LCE
    CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_EXIF,"adaptive LCE:");
    setIspIdx(rTags,  IDX_LCE              ,  (MUINT32)m_pLceMgr->getIDX());
    setIspIdx(rTags,  SW_LCE_ChipVersion      ,  m_LceExifInfo.i4ChipVersion);
    setIspIdx(rTags,  SW_LCE_MainVersion      ,  m_LceExifInfo.i4MainVersion);
    setIspIdx(rTags,  SW_LCE_SubVersion       ,  m_LceExifInfo.i4SubVersion);
    setIspIdx(rTags,  SW_LCE_SystemVersion    ,  m_LceExifInfo.i4SystemVersion);
    setIspIdx(rTags,  SW_LCE_PatchVersion     ,  m_LceExifInfo.i4PatchVersion);
    setIspIdx(rTags,  SW_LCE_LV               ,  m_LceExifInfo.i4LV);
    setIspIdx(rTags,  SW_LCE_LVIdx_L          ,  m_LceExifInfo.i4LVIdx_L);
    setIspIdx(rTags,  SW_LCE_LVIdx_H          ,  m_LceExifInfo.i4LVIdx_H);
    setIspIdx(rTags,  SW_LCE_CurrDR           ,  m_LceExifInfo.i4CurrDR);
    setIspIdx(rTags,  SW_LCE_DRIdx_L          ,  m_LceExifInfo.i4DRIdx_L);
    setIspIdx(rTags,  SW_LCE_DRIdx_H          ,  m_LceExifInfo.i4DRIdx_H);
    setIspIdx(rTags,  SW_LCE_DiffRangeIdx_L   ,  m_LceExifInfo.i4DiffRangeIdx_L);
    setIspIdx(rTags,  SW_LCE_DiffRangeIdx_H   ,  m_LceExifInfo.i4DiffRangeIdx_H);
    setIspIdx(rTags,  SW_LCE_LumaTarget       ,  m_LceExifInfo.i4LumaTarget);
    setIspIdx(rTags,  SW_LCE_FlatTarget       ,  m_LceExifInfo.i4FlatTarget);
    setIspIdx(rTags,  SW_LCE_LumaProb         ,  m_LceExifInfo.i4LumaProb);
    setIspIdx(rTags,  SW_LCE_FlatProb         ,  m_LceExifInfo.i4FlatProb);
    setIspIdx(rTags,  SW_LCE_LumaFlatProb     ,  m_LceExifInfo.i4LumaFlatProb);
    setIspIdx(rTags,  SW_LCE_LumaFlatTarget   ,  m_LceExifInfo.i4LumaFlatTarget);
    setIspIdx(rTags,  SW_LCE_LVTarget         ,  m_LceExifInfo.i4LVTarget);
    setIspIdx(rTags,  SW_LCE_BrightAvg        ,  m_LceExifInfo.i4BrightAvg);
    setIspIdx(rTags,  SW_LCE_LVProb           ,  m_LceExifInfo.i4LVProb);
    setIspIdx(rTags,  SW_LCE_DStrengthLevel   ,  m_LceExifInfo.i4DStrengthLevel);
    setIspIdx(rTags,  SW_LCE_BStrengthLevel   ,  m_LceExifInfo.i4BStrengthLevel);
    setIspIdx(rTags,  SW_LCE_FinalTarget      ,  m_LceExifInfo.i4FinalTarget);
    setIspIdx(rTags,  SW_LCE_FinalDStrength   ,  m_LceExifInfo.i4FinalDStrength);
    setIspIdx(rTags,  SW_LCE_FinalBStrength   ,  m_LceExifInfo.i4FinalBStrength);
    setIspIdx(rTags,  SW_LCE_P0               ,  m_LceExifInfo.i4P0);
    setIspIdx(rTags,  SW_LCE_P1               ,  m_LceExifInfo.i4P1);
    setIspIdx(rTags,  SW_LCE_P50              ,  m_LceExifInfo.i4P50);
    setIspIdx(rTags,  SW_LCE_P250             ,  m_LceExifInfo.i4P250);
    setIspIdx(rTags,  SW_LCE_P500             ,  m_LceExifInfo.i4P500);
    setIspIdx(rTags,  SW_LCE_P750             ,  m_LceExifInfo.i4P750);
    setIspIdx(rTags,  SW_LCE_P950             ,  m_LceExifInfo.i4P950);
    setIspIdx(rTags,  SW_LCE_P999             ,  m_LceExifInfo.i4P999);
    setIspIdx(rTags,  SW_LCE_O0               ,  m_LceExifInfo.i4O0);
    setIspIdx(rTags,  SW_LCE_O1               ,  m_LceExifInfo.i4O1);
    setIspIdx(rTags,  SW_LCE_O50              ,  m_LceExifInfo.i4O50);
    setIspIdx(rTags,  SW_LCE_O250             ,  m_LceExifInfo.i4O250);
    setIspIdx(rTags,  SW_LCE_O500             ,  m_LceExifInfo.i4O500);
    setIspIdx(rTags,  SW_LCE_O750             ,  m_LceExifInfo.i4O750);
    setIspIdx(rTags,  SW_LCE_O950             ,  m_LceExifInfo.i4O950);
    setIspIdx(rTags,  SW_LCE_O999             ,  m_LceExifInfo.i4O999);
    setIspIdx(rTags,  SW_LCE_CenMaxSlope      ,  m_LceExifInfo.i4CenMaxSlope);
    setIspIdx(rTags,  SW_LCE_CenMinSlope      ,  m_LceExifInfo.i4CenMinSlope);
    setIspIdx(rTags,  SW_LCE_LCE_FD_Enable    ,  m_LceExifInfo.i4LCE_FD_Enable);
    setIspIdx(rTags,  SW_LCE_FaceNum          ,  m_LceExifInfo.i4FaceNum);
    setIspIdx(rTags,  SW_LCE_FDXLow           ,  m_LceExifInfo.i4FDXLow);
    setIspIdx(rTags,  SW_LCE_FDXHi            ,  m_LceExifInfo.i4FDXHi);
    setIspIdx(rTags,  SW_LCE_FDYLow           ,  m_LceExifInfo.i4FDYLow);
    setIspIdx(rTags,  SW_LCE_FDYHi            ,  m_LceExifInfo.i4FDYHi);
    setIspIdx(rTags,  SW_LCE_LCSXLow          ,  m_LceExifInfo.i4LCSXLow);
    setIspIdx(rTags,  SW_LCE_LCSXHi           ,  m_LceExifInfo.i4LCSXHi);
    setIspIdx(rTags,  SW_LCE_LCSYLow          ,  m_LceExifInfo.i4LCSYLow);
    setIspIdx(rTags,  SW_LCE_LCSYHi           ,  m_LceExifInfo.i4LCSYHi);
    setIspIdx(rTags,  SW_LCE_FaceLoBound      ,  m_LceExifInfo.i4FaceLoBound);
    setIspIdx(rTags,  SW_LCE_FaceHiBound      ,  m_LceExifInfo.i4FaceHiBound);
    setIspIdx(rTags,  SW_LCE_ctrlPoint_f0     ,  m_LceExifInfo.i4ctrlPoint_f0);
    setIspIdx(rTags,  SW_LCE_ctrlPoint_f1     ,  m_LceExifInfo.i4ctrlPoint_f1);
    setIspIdx(rTags,  SW_LCE_FDY              ,  m_LceExifInfo.i4FDY);
    setIspIdx(rTags,  SW_LCE_MeterFDTarget    ,  m_LceExifInfo.i4MeterFDTarget);
    setIspIdx(rTags,  SW_LCE_MeterFDLinkTarget,  m_LceExifInfo.i4MeterFDLinkTarget);
    setIspIdx(rTags,  SW_LCE_AEGain           ,  m_LceExifInfo.i4AEGain);
    setIspIdx(rTags,  SW_LCE_Face_AELinkEnable,  m_LceExifInfo.i4FaceAELCELinkEnable);
    setIspIdx(rTags,  SW_LCE_MaxAEGain        ,  m_LceExifInfo.i4MaxAEGain);
    setIspIdx(rTags,  SW_LCE_IsFrontLight     ,  m_LceExifInfo.bIsFrontLight);
    setIspIdx(rTags,  SW_LCE_AETouchEnable    ,  m_LceExifInfo.bAETouchEnable);
    setIspIdx(rTags,  SW_LCE_FaceAEEnable     ,  m_LceExifInfo.bFaceAEEnable);
    setIspIdx(rTags,  SW_LCE_OriP0            ,  m_LceExifInfo.i4OriP0);
    setIspIdx(rTags,  SW_LCE_OriP1            ,  m_LceExifInfo.i4OriP1);
    setIspIdx(rTags,  SW_LCE_OriP50           ,  m_LceExifInfo.i4OriP50);
    setIspIdx(rTags,  SW_LCE_OriP250          ,  m_LceExifInfo.i4OriP250);
    setIspIdx(rTags,  SW_LCE_OriP500          ,  m_LceExifInfo.i4OriP500);
    setIspIdx(rTags,  SW_LCE_OriP750          ,  m_LceExifInfo.i4OriP750);
    setIspIdx(rTags,  SW_LCE_OriP950          ,  m_LceExifInfo.i4OriP950);
    setIspIdx(rTags,  SW_LCE_OriP999          ,  m_LceExifInfo.i4OriP999);
    setIspIdx(rTags,  SW_LCE_OriO0            ,  m_LceExifInfo.i4OriO0);
    setIspIdx(rTags,  SW_LCE_OriO1            ,  m_LceExifInfo.i4OriO1);
    setIspIdx(rTags,  SW_LCE_OriO50           ,  m_LceExifInfo.i4OriO50);
    setIspIdx(rTags,  SW_LCE_OriO250          ,  m_LceExifInfo.i4OriO250);
    setIspIdx(rTags,  SW_LCE_OriO500          ,  m_LceExifInfo.i4OriO500);
    setIspIdx(rTags,  SW_LCE_OriO750          ,  m_LceExifInfo.i4OriO750);
    setIspIdx(rTags,  SW_LCE_OriO950          ,  m_LceExifInfo.i4OriO950);
    setIspIdx(rTags,  SW_LCE_OriO999          ,  m_LceExifInfo.i4OriO999);
    setIspIdx(rTags,  SW_LCE_FaceP0           ,  m_LceExifInfo.i4FaceP0);
    setIspIdx(rTags,  SW_LCE_FaceP1           ,  m_LceExifInfo.i4FaceP1);
    setIspIdx(rTags,  SW_LCE_FaceP50          ,  m_LceExifInfo.i4FaceP50);
    setIspIdx(rTags,  SW_LCE_FaceP250         ,  m_LceExifInfo.i4FaceP250);
    setIspIdx(rTags,  SW_LCE_FaceP500         ,  m_LceExifInfo.i4FaceP500);
    setIspIdx(rTags,  SW_LCE_FaceP750         ,  m_LceExifInfo.i4FaceP750);
    setIspIdx(rTags,  SW_LCE_FaceP950         ,  m_LceExifInfo.i4FaceP950);
    setIspIdx(rTags,  SW_LCE_FaceP999         ,  m_LceExifInfo.i4FaceP999);
    setIspIdx(rTags,  SW_LCE_FaceO0           ,  m_LceExifInfo.i4FaceO0);
    setIspIdx(rTags,  SW_LCE_FaceO1           ,  m_LceExifInfo.i4FaceO1);
    setIspIdx(rTags,  SW_LCE_FaceO50          ,  m_LceExifInfo.i4FaceO50);
    setIspIdx(rTags,  SW_LCE_FaceO250         ,  m_LceExifInfo.i4FaceO250);
    setIspIdx(rTags,  SW_LCE_FaceO500         ,  m_LceExifInfo.i4FaceO500);
    setIspIdx(rTags,  SW_LCE_FaceO750         ,  m_LceExifInfo.i4FaceO750);
    setIspIdx(rTags,  SW_LCE_FaceO950         ,  m_LceExifInfo.i4FaceO950);
    setIspIdx(rTags,  SW_LCE_FaceO999         ,  m_LceExifInfo.i4FaceO999);
    setIspIdx(rTags,  SW_LCE_FaceSmothLinkEnable,  m_LceExifInfo.bFaceSmoothLinkEnable);
    setIspIdx(rTags,  SW_LCE_FaceState        ,  m_LceExifInfo.u4FaceState);
    setIspIdx(rTags,  SW_LCE_FaceRobustCnt    ,  m_LceExifInfo.u4FaceRobustCnt);
    setIspIdx(rTags,  SW_LCE_FaceRobustTrustCnt,  m_LceExifInfo.u4FaceRobustTrustCnt);
    setIspIdx(rTags,  SW_LCE_NonRobustFaceCnt ,  m_LceExifInfo.u4NonRobustFaceCnt);
    setIspIdx(rTags,  SW_LCE_FaceLCEStable    ,  m_LceExifInfo.bFaceLCEStable);
    setIspIdx(rTags,  SW_LCE_FaceStableCnt    ,  m_LceExifInfo.u4FaceStableCnt);
    setIspIdx(rTags,  SW_LCE_FaceStableInThd  ,  m_LceExifInfo.u4FaceInStableThd);
    setIspIdx(rTags,  SW_LCE_FaceStableOutThd ,  m_LceExifInfo.u4FaceOutStableThd);
    setIspIdx(rTags,  SW_LCE_NeedFaceProtection,  m_LceExifInfo.u4NeedFaceProtection);
    setIspIdx(rTags,  SW_LCE_NeedFaceGain     ,  m_LceExifInfo.u4NeedFaceGain);
    setIspIdx(rTags,  SW_LCE_FaceProtectMode  ,  m_LceExifInfo.u4FaceProtectMode);
    setIspIdx(rTags,  SW_LCE_SmoothEnable     ,  m_LceExifInfo.i4SmoothEnable);
    setIspIdx(rTags,  SW_LCE_LCEPosSpeed      ,  m_LceExifInfo.i4LCEPosSpeed);
    setIspIdx(rTags,  SW_LCE_LCENegSpeed      ,  m_LceExifInfo.i4LCENegSpeed);
    setIspIdx(rTags,  SW_LCE_LCECrosSpeed     ,  m_LceExifInfo.i4LCECrosSpeed);
    setIspIdx(rTags,  SW_LCE_LCESpeed4AEStable,  m_LceExifInfo.i4LCESpeed4AEStable);
    setIspIdx(rTags,  SW_LCE_DStrengthRatio   ,  m_LceExifInfo.i4DStrengthRatio);
    setIspIdx(rTags,  SW_LCE_BStrengthRatio   ,  m_LceExifInfo.i4BStrengthRatio);
    setIspIdx(rTags,  SW_LCE_CurrMaxLceGain   ,  m_LceExifInfo.i4CurrMaxLceGain);
    setIspIdx(rTags,  SW_LCE_MaxFinalTarget   ,  m_LceExifInfo.i4MaxFinalTarget);
    setIspIdx(rTags,  SW_LCE_LumaMean         ,  m_LceExifInfo.i4LumaMean);
    setIspIdx(rTags,  SW_LCE_FlatMean         ,  m_LceExifInfo.i4FlatMean);
    setIspIdx(rTags,  SW_LCE_DSTDIndex        ,  m_LceExifInfo.i4DSTDIndex);
    setIspIdx(rTags,  SW_LCE_DarkSkyProtEnable,  m_LceExifInfo.bDarkSkyProtectEnable);
    setIspIdx(rTags,  SW_LCE_DarkFlatY        ,  m_LceExifInfo.i4DarkFlatY);
    setIspIdx(rTags,  SW_LCE_DarkFlatRatio    ,  m_LceExifInfo.i4DarkFlatRatio);
    setIspIdx(rTags,  SW_LCE_FlatLVRatio      ,  m_LceExifInfo.i4FlatLVRatio);
    setIspIdx(rTags,  SW_LCE_BypassLCE        ,  m_LceExifInfo.bBypassLCE);
    setIspIdx(rTags,  SW_LCE_IncorrectLCSO    ,  m_LceExifInfo.bIncorrectLCSO);
    setIspIdx(rTags,  SW_LCE_GCEP0            ,  m_LceExifInfo.i4GCEP0);
    setIspIdx(rTags,  SW_LCE_GCEP1            ,  m_LceExifInfo.i4GCEP1);
    setIspIdx(rTags,  SW_LCE_GCEP50           ,  m_LceExifInfo.i4GCEP50);
    setIspIdx(rTags,  SW_LCE_GCEP250          ,  m_LceExifInfo.i4GCEP250);
    setIspIdx(rTags,  SW_LCE_GCEP500          ,  m_LceExifInfo.i4GCEP500);
    setIspIdx(rTags,  SW_LCE_GCEP750          ,  m_LceExifInfo.i4GCEP750);
    setIspIdx(rTags,  SW_LCE_GCEP950          ,  m_LceExifInfo.i4GCEP950);
    setIspIdx(rTags,  SW_LCE_GCEP999          ,  m_LceExifInfo.i4GCEP999);
    setIspIdx(rTags,  SW_LCE_GCEO0            ,  m_LceExifInfo.i4GCEO0);
    setIspIdx(rTags,  SW_LCE_GCEO1            ,  m_LceExifInfo.i4GCEO1);
    setIspIdx(rTags,  SW_LCE_GCEO50           ,  m_LceExifInfo.i4GCEO50);
    setIspIdx(rTags,  SW_LCE_GCEO250          ,  m_LceExifInfo.i4GCEO250);
    setIspIdx(rTags,  SW_LCE_GCEO500          ,  m_LceExifInfo.i4GCEO500);
    setIspIdx(rTags,  SW_LCE_GCEO750          ,  m_LceExifInfo.i4GCEO750);
    setIspIdx(rTags,  SW_LCE_GCEO950          ,  m_LceExifInfo.i4GCEO950);
    setIspIdx(rTags,  SW_LCE_GCEO999          ,  m_LceExifInfo.i4GCEO999);
    setIspIdx(rTags,  SW_LCE_AEMagicNum       ,  m_LceExifInfo.i4AEMagicNum);


    setIspIdx(rTags, IDX_DCE, (MUINT32)m_pDceMgr->getIDX());
    setIspIdx(rTags, SW_DCE_chip_version,         m_DceExifInfo.chip_version );
    setIspIdx(rTags, SW_DCE_main_version,         m_DceExifInfo.main_version );
    setIspIdx(rTags, SW_DCE_sub_version,          m_DceExifInfo.sub_version );
    setIspIdx(rTags, SW_DCE_system_version,       m_DceExifInfo.system_version );
    setIspIdx(rTags, SW_DCE_lv_idx_l,             m_DceExifInfo.lv_idx_l );
    setIspIdx(rTags, SW_DCE_lv_idx_h,             m_DceExifInfo.lv_idx_h );
    setIspIdx(rTags, SW_DCE_dr_idx_l,             m_DceExifInfo.dr_idx_l );
    setIspIdx(rTags, SW_DCE_dr_idx_h,             m_DceExifInfo.dr_idx_h );
    setIspIdx(rTags, SW_DCE_dce_flat_ratio_thd_l, m_DceExifInfo.dce_flat_ratio_thd_l );
    setIspIdx(rTags, SW_DCE_dce_flat_ratio_thd_h, m_DceExifInfo.dce_flat_ratio_thd_h );
    setIspIdx(rTags, SW_DCE_dce_smooth_speed,     m_DceExifInfo.dce_smooth_speed );
    setIspIdx(rTags, SW_DCE_dce_p1,   m_DceExifInfo.dce_p1 );
    setIspIdx(rTags, SW_DCE_dce_p2,   m_DceExifInfo.dce_p2 );
    setIspIdx(rTags, SW_DCE_dce_p3,   m_DceExifInfo.dce_p3 );
    setIspIdx(rTags, SW_DCE_dce_p4,   m_DceExifInfo.dce_p4 );
    setIspIdx(rTags, SW_DCE_dce_p5,   m_DceExifInfo.dce_p5 );
    setIspIdx(rTags, SW_DCE_dce_p6,   m_DceExifInfo.dce_p6 );
    setIspIdx(rTags, SW_DCE_dce_p7,   m_DceExifInfo.dce_p7 );
    setIspIdx(rTags, SW_DCE_dce_p8,   m_DceExifInfo.dce_p8 );
    setIspIdx(rTags, SW_DCE_dce_p9,   m_DceExifInfo.dce_p9 );
    setIspIdx(rTags, SW_DCE_dce_p10,  m_DceExifInfo.dce_p10 );
    setIspIdx(rTags, SW_DCE_dce_p11,  m_DceExifInfo.dce_p11 );
    setIspIdx(rTags, SW_DCE_dce_p12,  m_DceExifInfo.dce_p12 );
    setIspIdx(rTags, SW_DCE_dce_p13,  m_DceExifInfo.dce_p13 );
    setIspIdx(rTags, SW_DCE_dce_p14,  m_DceExifInfo.dce_p14 );
    setIspIdx(rTags, SW_DCE_dce_p15,  m_DceExifInfo.dce_p15 );
    setIspIdx(rTags, SW_DCE_dce_p16,  m_DceExifInfo.dce_p16 );
    setIspIdx(rTags, SW_DCE_dce_p17,  m_DceExifInfo.dce_p17 );
    setIspIdx(rTags, SW_DCE_dce_p18,  m_DceExifInfo.dce_p18 );
    setIspIdx(rTags, SW_DCE_dce_p19,  m_DceExifInfo.dce_p19 );
    setIspIdx(rTags, SW_DCE_dce_p20,  m_DceExifInfo.dce_p20 );
    setIspIdx(rTags, SW_DCE_dce_p21,  m_DceExifInfo.dce_p21 );
    setIspIdx(rTags, SW_DCE_dce_p22,  m_DceExifInfo.dce_p22 );
    setIspIdx(rTags, SW_DCE_dce_p23,  m_DceExifInfo.dce_p23 );
    setIspIdx(rTags, SW_DCE_dce_p24,  m_DceExifInfo.dce_p24 );
    setIspIdx(rTags, SW_DCE_dce_p25,  m_DceExifInfo.dce_p25 );
    setIspIdx(rTags, SW_DCE_dce_p26,  m_DceExifInfo.dce_p26 );
    setIspIdx(rTags, SW_DCE_dce_p27,  m_DceExifInfo.dce_p27 );
    setIspIdx(rTags, SW_DCE_dce_p28,  m_DceExifInfo.dce_p28 );
    setIspIdx(rTags, SW_DCE_dce_p29,  m_DceExifInfo.dce_p29 );
    setIspIdx(rTags, SW_DCE_dce_p30,  m_DceExifInfo.dce_p30 );
    setIspIdx(rTags, SW_DCE_dce_p31,  m_DceExifInfo.dce_p31 );
    setIspIdx(rTags, SW_DCE_dce_p32,  m_DceExifInfo.dce_p32 );
    setIspIdx(rTags, SW_DCE_dce_p33,  m_DceExifInfo.dce_p33 );
    setIspIdx(rTags, SW_DCE_dce_p34,  m_DceExifInfo.dce_p34 );
    setIspIdx(rTags, SW_DCE_dce_p35,  m_DceExifInfo.dce_p35 );
    setIspIdx(rTags, SW_DCE_dce_p36,  m_DceExifInfo.dce_p36 );
    setIspIdx(rTags, SW_DCE_dce_p37,  m_DceExifInfo.dce_p37 );
    setIspIdx(rTags, SW_DCE_dce_p38,  m_DceExifInfo.dce_p38 );
    setIspIdx(rTags, SW_DCE_dce_p39,  m_DceExifInfo.dce_p39 );
    setIspIdx(rTags, SW_DCE_dce_p40,  m_DceExifInfo.dce_p40 );
    setIspIdx(rTags, SW_DCE_dce_p41,  m_DceExifInfo.dce_p41 );
    setIspIdx(rTags, SW_DCE_dce_p42,  m_DceExifInfo.dce_p42 );
    setIspIdx(rTags, SW_DCE_dce_p43,  m_DceExifInfo.dce_p43 );
    setIspIdx(rTags, SW_DCE_dce_p44,  m_DceExifInfo.dce_p44 );
    setIspIdx(rTags, SW_DCE_dce_p45,  m_DceExifInfo.dce_p45 );
    setIspIdx(rTags, SW_DCE_dce_p46,  m_DceExifInfo.dce_p46 );
    setIspIdx(rTags, SW_DCE_dce_p47,  m_DceExifInfo.dce_p47 );
    setIspIdx(rTags, SW_DCE_dce_p48,  m_DceExifInfo.dce_p48 );
    setIspIdx(rTags, SW_DCE_dce_p49,  m_DceExifInfo.dce_p49 );
    setIspIdx(rTags, SW_DCE_dce_p50,  m_DceExifInfo.dce_p50 );
    setIspIdx(rTags, SW_DCE_dce_p51,  m_DceExifInfo.dce_p51 );
    setIspIdx(rTags, SW_DCE_dce_p52,  m_DceExifInfo.dce_p52 );
    setIspIdx(rTags, SW_DCE_dce_p53,  m_DceExifInfo.dce_p53 );
    setIspIdx(rTags, SW_DCE_dce_p54,  m_DceExifInfo.dce_p54 );
    setIspIdx(rTags, SW_DCE_dce_p55,  m_DceExifInfo.dce_p55 );
    setIspIdx(rTags, SW_DCE_dce_p56,  m_DceExifInfo.dce_p56 );
    setIspIdx(rTags, SW_DCE_dce_p57,  m_DceExifInfo.dce_p57 );
    setIspIdx(rTags, SW_DCE_dce_p58,  m_DceExifInfo.dce_p58 );
    setIspIdx(rTags, SW_DCE_dce_p59,  m_DceExifInfo.dce_p59 );
    setIspIdx(rTags, SW_DCE_dce_p60,  m_DceExifInfo.dce_p60 );
    setIspIdx(rTags, SW_DCE_dce_p61,  m_DceExifInfo.dce_p61 );
    setIspIdx(rTags, SW_DCE_dce_p62,  m_DceExifInfo.dce_p62 );
    setIspIdx(rTags, SW_DCE_dce_p63,  m_DceExifInfo.dce_p63 );
    setIspIdx(rTags, SW_DCE_dce_p64,  m_DceExifInfo.dce_p64 );
    setIspIdx(rTags, SW_DCE_flare_offset, m_DceExifInfo.flare_offset );
    setIspIdx(rTags, SW_DCE_flare_gain,   m_DceExifInfo.flare_gain );
    setIspIdx(rTags, SW_DCE_dce_pt0,      m_DceExifInfo.dce_pt0 );
    setIspIdx(rTags, SW_DCE_dce_pt1,      m_DceExifInfo.dce_pt1 );
    setIspIdx(rTags, SW_DCE_dce_pt2,      m_DceExifInfo.dce_pt2 );
    setIspIdx(rTags, SW_DCE_dce_pt3,      m_DceExifInfo.dce_pt3 );
    setIspIdx(rTags, SW_DCE_dce_sub_hist1_height_max,   m_DceExifInfo.dce_sub_hist1_height_max);
    setIspIdx(rTags, SW_DCE_dce_sub_hist1_height_min,   m_DceExifInfo.dce_sub_hist1_height_min);
    setIspIdx(rTags, SW_DCE_dce_plat1,				   m_DceExifInfo.dce_plat1);
    setIspIdx(rTags, SW_DCE_dce_sub_hist2_height_max,   m_DceExifInfo.dce_sub_hist2_height_max);
    setIspIdx(rTags, SW_DCE_dce_sub_hist2_height_min,   m_DceExifInfo.dce_sub_hist2_height_min);
    setIspIdx(rTags, SW_DCE_dce_plat2,				   m_DceExifInfo.dce_plat2);
    setIspIdx(rTags, SW_DCE_dce_sub_hist3_height_max,   m_DceExifInfo.dce_sub_hist3_height_max);
    setIspIdx(rTags, SW_DCE_dce_sub_hist3_height_min,   m_DceExifInfo.dce_sub_hist3_height_min);
    setIspIdx(rTags, SW_DCE_dce_plat3,				   m_DceExifInfo.dce_plat3);
    setIspIdx(rTags, SW_DCE_dce_str1,     m_DceExifInfo.dce_str1 );
    setIspIdx(rTags, SW_DCE_dce_str2,     m_DceExifInfo.dce_str2 );
    setIspIdx(rTags, SW_DCE_dce_str3,     m_DceExifInfo.dce_str3 );
    setIspIdx(rTags, SW_DCE_dce_flat_prot_flag1, 	   m_DceExifInfo.dce_flat_prot_flag1);
    setIspIdx(rTags, SW_DCE_dce_flat_prot_flag2, 	   m_DceExifInfo.dce_flat_prot_flag2);
    setIspIdx(rTags, SW_DCE_dce_flat_prot_flag3, 	   m_DceExifInfo.dce_flat_prot_flag3);
    setIspIdx(rTags, SW_DCE_dces_en, 				   m_DceExifInfo.dces_en);
    setIspIdx(rTags, SW_DCE_get_default, 			   m_DceExifInfo.get_default);
    setIspIdx(rTags, SW_DCE_dce_slope_max,			   m_DceExifInfo.dce_slope_max);
    setIspIdx(rTags, SW_DCE_smooth_speed,			   m_DceExifInfo.smooth_speed);
    setIspIdx(rTags, SW_DCE_dce_flat_prot_flag,		   m_DceExifInfo.dce_flat_prot_flag);

    setIspIdx(rTags,  FACE_NUM,                 rCamInfo.rFdInfo.FaceNum);
    setIspIdx(rTags,  FACE_GENDER_NUM,          rCamInfo.rFdInfo.GenderNum);
    setIspIdx(rTags,  FACE_LandmarkNum,         rCamInfo.rFdInfo.LandmarkNum);

    setIspIdx(rTags,  FACE_1_1,                 rCamInfo.rFdInfo.YUVsts[0][0]);
    setIspIdx(rTags,  FACE_1_2,                 rCamInfo.rFdInfo.YUVsts[0][1]);
    setIspIdx(rTags,  FACE_1_3,                 rCamInfo.rFdInfo.YUVsts[0][2]);
    setIspIdx(rTags,  FACE_1_4,                 rCamInfo.rFdInfo.YUVsts[0][3]);
    setIspIdx(rTags,  FACE_1_5,                 rCamInfo.rFdInfo.YUVsts[0][4]);
    setIspIdx(rTags,  FACE_1_fld_GenderLabel,   rCamInfo.rFdInfo.fld_GenderLabel[0]);
    setIspIdx(rTags,  FACE_1_fld_GenderInfo,    rCamInfo.rFdInfo.fld_GenderInfo[0]);
    setIspIdx(rTags,  FACE_1_fld_rip,           rCamInfo.rFdInfo.fld_rip[0]);
    setIspIdx(rTags,  FACE_1_fld_rop,           rCamInfo.rFdInfo.fld_rop[0]);
    setIspIdx(rTags,  FACE_1_rect_1,            rCamInfo.rFdInfo.rect[0][0]);
    setIspIdx(rTags,  FACE_1_rect_2,            rCamInfo.rFdInfo.rect[0][1]);
    setIspIdx(rTags,  FACE_1_rect_3,            rCamInfo.rFdInfo.rect[0][2]);
    setIspIdx(rTags,  FACE_1_rect_4,            rCamInfo.rFdInfo.rect[0][3]);
    setIspIdx(rTags,  FACE_1_Face_Leye_1,       rCamInfo.rFdInfo.Face_Leye[0][0]);
    setIspIdx(rTags,  FACE_1_Face_Leye_2,       rCamInfo.rFdInfo.Face_Leye[0][1]);
    setIspIdx(rTags,  FACE_1_Face_Leye_3,       rCamInfo.rFdInfo.Face_Leye[0][2]);
    setIspIdx(rTags,  FACE_1_Face_Leye_4,       rCamInfo.rFdInfo.Face_Leye[0][3]);
    setIspIdx(rTags,  FACE_1_Face_Reye_1,       rCamInfo.rFdInfo.Face_Reye[0][0]);
    setIspIdx(rTags,  FACE_1_Face_Reye_2,       rCamInfo.rFdInfo.Face_Reye[0][1]);
    setIspIdx(rTags,  FACE_1_Face_Reye_3,       rCamInfo.rFdInfo.Face_Reye[0][2]);
    setIspIdx(rTags,  FACE_1_Face_Reye_4,       rCamInfo.rFdInfo.Face_Reye[0][3]);
    setIspIdx(rTags,  FACE_1_Landmark_CV,       rCamInfo.rFdInfo.Landmark_CV[0]);


    setIspIdx(rTags,  FACE_2_1,                 rCamInfo.rFdInfo.YUVsts[1][0]);
    setIspIdx(rTags,  FACE_2_2,                 rCamInfo.rFdInfo.YUVsts[1][1]);
    setIspIdx(rTags,  FACE_2_3,                 rCamInfo.rFdInfo.YUVsts[1][2]);
    setIspIdx(rTags,  FACE_2_4,                 rCamInfo.rFdInfo.YUVsts[1][3]);
    setIspIdx(rTags,  FACE_2_5,                 rCamInfo.rFdInfo.YUVsts[1][4]);
    setIspIdx(rTags,  FACE_2_fld_GenderLabel,   rCamInfo.rFdInfo.fld_GenderLabel[1]);
    setIspIdx(rTags,  FACE_2_fld_GenderInfo,    rCamInfo.rFdInfo.fld_GenderInfo[1]);
    setIspIdx(rTags,  FACE_2_fld_rip,           rCamInfo.rFdInfo.fld_rip[1]);
    setIspIdx(rTags,  FACE_2_fld_rop,           rCamInfo.rFdInfo.fld_rop[1]);
    setIspIdx(rTags,  FACE_2_rect_1,            rCamInfo.rFdInfo.rect[1][0]);
    setIspIdx(rTags,  FACE_2_rect_2,            rCamInfo.rFdInfo.rect[1][1]);
    setIspIdx(rTags,  FACE_2_rect_3,            rCamInfo.rFdInfo.rect[1][2]);
    setIspIdx(rTags,  FACE_2_rect_4,            rCamInfo.rFdInfo.rect[1][3]);
    setIspIdx(rTags,  FACE_2_Face_Leye_1,       rCamInfo.rFdInfo.Face_Leye[1][0]);
    setIspIdx(rTags,  FACE_2_Face_Leye_2,       rCamInfo.rFdInfo.Face_Leye[1][1]);
    setIspIdx(rTags,  FACE_2_Face_Leye_3,       rCamInfo.rFdInfo.Face_Leye[1][2]);
    setIspIdx(rTags,  FACE_2_Face_Leye_4,       rCamInfo.rFdInfo.Face_Leye[1][3]);
    setIspIdx(rTags,  FACE_2_Face_Reye_1,       rCamInfo.rFdInfo.Face_Reye[1][0]);
    setIspIdx(rTags,  FACE_2_Face_Reye_2,       rCamInfo.rFdInfo.Face_Reye[1][1]);
    setIspIdx(rTags,  FACE_2_Face_Reye_3,       rCamInfo.rFdInfo.Face_Reye[1][2]);
    setIspIdx(rTags,  FACE_2_Face_Reye_4,       rCamInfo.rFdInfo.Face_Reye[1][3]);
    setIspIdx(rTags,  FACE_2_Landmark_CV,       rCamInfo.rFdInfo.Landmark_CV[1]);

    setIspIdx(rTags,  FACE_3_1,                 rCamInfo.rFdInfo.YUVsts[2][0]);
    setIspIdx(rTags,  FACE_3_2,                 rCamInfo.rFdInfo.YUVsts[2][1]);
    setIspIdx(rTags,  FACE_3_3,                 rCamInfo.rFdInfo.YUVsts[2][2]);
    setIspIdx(rTags,  FACE_3_4,                 rCamInfo.rFdInfo.YUVsts[2][3]);
    setIspIdx(rTags,  FACE_3_5,                 rCamInfo.rFdInfo.YUVsts[2][4]);
    setIspIdx(rTags,  FACE_3_fld_GenderLabel,   rCamInfo.rFdInfo.fld_GenderLabel[2]);
    setIspIdx(rTags,  FACE_3_fld_GenderInfo,    rCamInfo.rFdInfo.fld_GenderInfo[2]);
    setIspIdx(rTags,  FACE_3_fld_rip,           rCamInfo.rFdInfo.fld_rip[2]);
    setIspIdx(rTags,  FACE_3_fld_rop,           rCamInfo.rFdInfo.fld_rop[2]);
    setIspIdx(rTags,  FACE_3_rect_1,            rCamInfo.rFdInfo.rect[2][0]);
    setIspIdx(rTags,  FACE_3_rect_2,            rCamInfo.rFdInfo.rect[2][1]);
    setIspIdx(rTags,  FACE_3_rect_3,            rCamInfo.rFdInfo.rect[2][2]);
    setIspIdx(rTags,  FACE_3_rect_4,            rCamInfo.rFdInfo.rect[2][3]);
    setIspIdx(rTags,  FACE_3_Face_Leye_1,       rCamInfo.rFdInfo.Face_Leye[2][0]);
    setIspIdx(rTags,  FACE_3_Face_Leye_2,       rCamInfo.rFdInfo.Face_Leye[2][1]);
    setIspIdx(rTags,  FACE_3_Face_Leye_3,       rCamInfo.rFdInfo.Face_Leye[2][2]);
    setIspIdx(rTags,  FACE_3_Face_Leye_4,       rCamInfo.rFdInfo.Face_Leye[2][3]);
    setIspIdx(rTags,  FACE_3_Face_Reye_1,       rCamInfo.rFdInfo.Face_Reye[2][0]);
    setIspIdx(rTags,  FACE_3_Face_Reye_2,       rCamInfo.rFdInfo.Face_Reye[2][1]);
    setIspIdx(rTags,  FACE_3_Face_Reye_3,       rCamInfo.rFdInfo.Face_Reye[2][2]);
    setIspIdx(rTags,  FACE_3_Face_Reye_4,       rCamInfo.rFdInfo.Face_Reye[2][3]);
    setIspIdx(rTags,  FACE_3_Landmark_CV,       rCamInfo.rFdInfo.Landmark_CV[2]);


    setIspIdx(rTags,  FACE_4_1,                 rCamInfo.rFdInfo.YUVsts[3][0]);
    setIspIdx(rTags,  FACE_4_2,                 rCamInfo.rFdInfo.YUVsts[3][1]);
    setIspIdx(rTags,  FACE_4_3,                 rCamInfo.rFdInfo.YUVsts[3][2]);
    setIspIdx(rTags,  FACE_4_4,                 rCamInfo.rFdInfo.YUVsts[3][3]);
    setIspIdx(rTags,  FACE_4_5,                 rCamInfo.rFdInfo.YUVsts[3][4]);
    setIspIdx(rTags,  FACE_4_fld_GenderLabel,   rCamInfo.rFdInfo.fld_GenderLabel[3]);
    setIspIdx(rTags,  FACE_4_fld_GenderInfo,    rCamInfo.rFdInfo.fld_GenderInfo[3]);
    setIspIdx(rTags,  FACE_4_fld_rip,           rCamInfo.rFdInfo.fld_rip[3]);
    setIspIdx(rTags,  FACE_4_fld_rop,           rCamInfo.rFdInfo.fld_rop[3]);
    setIspIdx(rTags,  FACE_4_rect_1,            rCamInfo.rFdInfo.rect[3][0]);
    setIspIdx(rTags,  FACE_4_rect_2,            rCamInfo.rFdInfo.rect[3][1]);
    setIspIdx(rTags,  FACE_4_rect_3,            rCamInfo.rFdInfo.rect[3][2]);
    setIspIdx(rTags,  FACE_4_rect_4,            rCamInfo.rFdInfo.rect[3][3]);
    setIspIdx(rTags,  FACE_4_Face_Leye_1,       rCamInfo.rFdInfo.Face_Leye[3][0]);
    setIspIdx(rTags,  FACE_4_Face_Leye_2,       rCamInfo.rFdInfo.Face_Leye[3][1]);
    setIspIdx(rTags,  FACE_4_Face_Leye_3,       rCamInfo.rFdInfo.Face_Leye[3][2]);
    setIspIdx(rTags,  FACE_4_Face_Leye_4,       rCamInfo.rFdInfo.Face_Leye[3][3]);
    setIspIdx(rTags,  FACE_4_Face_Reye_1,       rCamInfo.rFdInfo.Face_Reye[3][0]);
    setIspIdx(rTags,  FACE_4_Face_Reye_2,       rCamInfo.rFdInfo.Face_Reye[3][1]);
    setIspIdx(rTags,  FACE_4_Face_Reye_3,       rCamInfo.rFdInfo.Face_Reye[3][2]);
    setIspIdx(rTags,  FACE_4_Face_Reye_4,       rCamInfo.rFdInfo.Face_Reye[3][3]);
    setIspIdx(rTags,  FACE_4_Landmark_CV,       rCamInfo.rFdInfo.Landmark_CV[3]);

    setIspIdx(rTags,  FACE_5_1,                 rCamInfo.rFdInfo.YUVsts[4][0]);
    setIspIdx(rTags,  FACE_5_2,                 rCamInfo.rFdInfo.YUVsts[4][1]);
    setIspIdx(rTags,  FACE_5_3,                 rCamInfo.rFdInfo.YUVsts[4][2]);
    setIspIdx(rTags,  FACE_5_4,                 rCamInfo.rFdInfo.YUVsts[4][3]);
    setIspIdx(rTags,  FACE_5_5,                 rCamInfo.rFdInfo.YUVsts[4][4]);
    setIspIdx(rTags,  FACE_5_fld_GenderLabel,   rCamInfo.rFdInfo.fld_GenderLabel[4]);
    setIspIdx(rTags,  FACE_5_fld_GenderInfo,    rCamInfo.rFdInfo.fld_GenderInfo[4]);
    setIspIdx(rTags,  FACE_5_fld_rip,           rCamInfo.rFdInfo.fld_rip[4]);
    setIspIdx(rTags,  FACE_5_fld_rop,           rCamInfo.rFdInfo.fld_rop[4]);
    setIspIdx(rTags,  FACE_5_rect_1,            rCamInfo.rFdInfo.rect[4][0]);
    setIspIdx(rTags,  FACE_5_rect_2,            rCamInfo.rFdInfo.rect[4][1]);
    setIspIdx(rTags,  FACE_5_rect_3,            rCamInfo.rFdInfo.rect[4][2]);
    setIspIdx(rTags,  FACE_5_rect_4,            rCamInfo.rFdInfo.rect[4][3]);
    setIspIdx(rTags,  FACE_5_Face_Leye_1,       rCamInfo.rFdInfo.Face_Leye[4][0]);
    setIspIdx(rTags,  FACE_5_Face_Leye_2,       rCamInfo.rFdInfo.Face_Leye[4][1]);
    setIspIdx(rTags,  FACE_5_Face_Leye_3,       rCamInfo.rFdInfo.Face_Leye[4][2]);
    setIspIdx(rTags,  FACE_5_Face_Leye_4,       rCamInfo.rFdInfo.Face_Leye[4][3]);
    setIspIdx(rTags,  FACE_5_Face_Reye_1,       rCamInfo.rFdInfo.Face_Reye[4][0]);
    setIspIdx(rTags,  FACE_5_Face_Reye_2,       rCamInfo.rFdInfo.Face_Reye[4][1]);
    setIspIdx(rTags,  FACE_5_Face_Reye_3,       rCamInfo.rFdInfo.Face_Reye[4][2]);
    setIspIdx(rTags,  FACE_5_Face_Reye_4,       rCamInfo.rFdInfo.Face_Reye[4][3]);
    setIspIdx(rTags,  FACE_5_Landmark_CV,       rCamInfo.rFdInfo.Landmark_CV[4]);


    ////////////////////////////////////////////////////////////////////////////
    //  (2.2) ISPComm
    ////////////////////////////////////////////////////////////////////////////
    for (MUINT32 i = 0; i < sizeof(ISP_NVRAM_COMMON_STRUCT)/sizeof(MUINT32); i++)
    {
        MUINT32 const u4TagID = COMM_Begin + i;
        //rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = m_rIspComm.CommReg[i];
    }

    MUINT32 u4RegCnt_start = 0;
    MUINT32 u4RegCnt = 0;
    getP2ExifSizeInfo(u4RegCnt, u4RegCnt_start);

    if (u4RegCnt > P2_BUFFER_SIZE)
    {
        CAM_LOGE("P2 buffer:(%d) byte exceed EXIF allocation size:(%d) byte", u4RegCnt*4, P2_BUFFER_SIZE*4);
    }
    else
    {
        if (u4RegCnt_start != P2_START_ADDR_COUNT)
                CAM_LOGD_IF(m_bDebugEnable & PARAMCTL_LOG_EXIF,"[%s()] u4RegCnt_start(%d), P2_START_ADDR_COUNT(%d)",
                __FUNCTION__,u4RegCnt_start, P2_START_ADDR_COUNT);
        memcpy(rDebugInfo.P2RegInfo.regDataP2, (MINT32*)pRegBuf+u4RegCnt_start, u4RegCnt*4);
        rDebugInfo.P2RegInfo.u4TableSize = P2_BUFFER_SIZE+1;
        rDebugInfo.P2RegInfo.u4HwVersion = 9;
    }

    if(pMfbBuf){
        MUINT32 mfbSize = sizeof(mfb_reg_t);

        if (mfbSize/4 > MFB_BUFFER_SIZE)
        {
            CAM_LOGE("MFB buffer:(%d) byte exceed EXIF allocation size:(%d) byte", mfbSize, MFB_BUFFER_SIZE*4);
        }
        else
        {
            memcpy(rDebugInfo.MFBRegInfo.regDataMFB, pMfbBuf, mfbSize);
            rDebugInfo.MFBRegInfo.u4TableSize = MFB_BUFFER_SIZE+1;
            rDebugInfo.MFBRegInfo.u4HwVersion = 9;
        }
    }
#endif
    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setDGNDebugInfo4CCU(MUINT32 const u4Rto, MUINT32 const u4P1DGNGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    rDbgIspInfo.debugInfo.tags[CCU_DGN_Enable].u4Val = 1;

    MUINT32 DGN_Cnt;

    DGN_Cnt = ((MUINT32)offsetof(cam_reg_t, DGN_R1_DGN_GN0))/4;
    *(rDbgIspInfo.P1RegInfo.regData+DGN_Cnt) = u4P1DGNGain;

    DGN_Cnt = ((MUINT32)offsetof(cam_reg_t, DGN_R1_DGN_GN1))/4;
    *(rDbgIspInfo.P1RegInfo.regData+DGN_Cnt) = u4P1DGNGain;

    DGN_Cnt = ((MUINT32)offsetof(cam_reg_t, DGN_R1_DGN_GN2))/4;
    *(rDbgIspInfo.P1RegInfo.regData+DGN_Cnt) = u4P1DGNGain;

    DGN_Cnt = ((MUINT32)offsetof(cam_reg_t, DGN_R1_DGN_GN3))/4;
    *(rDbgIspInfo.P1RegInfo.regData+DGN_Cnt) = u4P1DGNGain;

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setHLRDebugInfo4CCU(ISP_NVRAM_HLR_T const HLR, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
/* HHHHH
    rDbgIspInfo.debugInfo.tags[CCU_HLR_Enable].u4Val = 1;

    if( ISP_MGR_HLR_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_HLR::EHLR_R1) )
    {
        MUINT32 CAMCTL_EN2_Cnt;
        CAMCTL_EN2_Cnt = ((MUINT32)offsetof(cam_reg_t, CAMCTL_R1_CAMCTL_EN2))/4;
        *(rDbgIspInfo.P1RegInfo.regData + CAMCTL_EN2_Cnt) |= 0x00000080;

        MUINT32 CAMCTL_HLR_Cnt;
        CAMCTL_HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, CAMCTL_R1_CAMCTL_HLR_LKMSB))/4;
        *(rDbgIspInfo.P1RegInfo.regData + CAMCTL_HLR_Cnt) = HLR.lkmsb.val;
    }


    MUINT32 HLR_Cnt;

    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_Y0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_y0.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_Y1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_y1.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_Y2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_y2.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_Y3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_y3.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_X0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_x0.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_X1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_x1.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_X2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_x2.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_X3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_x3.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_S0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_s0.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_S1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_s1.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EST_S2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.est_s2.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_LMG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.lmg.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_PRT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.prt.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_CLP))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.clp.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_EFCT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.efct.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_CTL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.ctl.val;
    HLR_Cnt = ((MUINT32)offsetof(cam_reg_t, HLR_R1_HLR_CTL2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + HLR_Cnt) = HLR.ctl2.val;
*/
    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setLTMnLTMSDebugInfo4CCU(ISP_NVRAM_LTM_T const LTM_R1, ISP_NVRAM_LTM_T const LTM_R2, ISP_NVRAM_LTMS_T const LTMS, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
/* HHHH
    rDbgIspInfo.debugInfo.tags[CCU_LTM_Enable].u4Val = 1;

    if( ISP_MGR_LTM_T::getInstance(m_eSensorDev).isEnable(ISP_MGR_LTM::ELTM_R1) )
    {
        MUINT32 CAMCTL_EN_Cnt;

        CAMCTL_EN_Cnt = ((MUINT32)offsetof(cam_reg_t, CAMCTL_R1_CAMCTL_EN))/4;
        *(rDbgIspInfo.P1RegInfo.regData + CAMCTL_EN_Cnt) |= 0x00008100;  //LTM
    }

    MUINT32 LTMS_Cnt;

    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_CTRL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.ctrl.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_BLK_NUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.blk_num.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_BLK_SZ))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.blk_sz.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_BLK_AREA))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.blk_area.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_DETAIL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.detail.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_HIST))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.hist.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_FLTLINE))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.fltline.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_FLTBLK))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.fltblk.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_CLIP))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.clip.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_MAX_DIV))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.max_div.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_CFG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.cfg.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_RESET))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.reset.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_INTEN))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.inten.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_INTSTA))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.intsta.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_STATUS))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.status.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_INPUT_COUNT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.input_count.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_OUTPUT_COUNT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.output_count.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_CHKSUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.chksum.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_IN_SIZE))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.in_size.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_OUT_SIZE))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.out_size.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_ACT_WINDOW_X))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.act_window_x.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_ACT_WINDOW_Y))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.act_window_y.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_OUT_DATA_NUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.out_data_num.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_DUMMY_REG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.dummy_reg.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_SRAM_CFG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.sram_cfg.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_ATPG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.atpg.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_SHADOW_CTRL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.shadow_ctrl.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_HIST_R))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.hist_r.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_HIST_B))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.hist_b.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_HIST_C))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.hist_c.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_FLATLINE_R))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.flatline_r.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_FLATBLK_B))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.flatblk_b.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_BLK_R_AREA))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.blk_r_area.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_BLK_B_AREA))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.blk_b_area.val;
    LTMS_Cnt = ((MUINT32)offsetof(cam_reg_t, LTMS_R1_LTMS_BLK_C_AREA))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTMS_Cnt) = LTMS.blk_c_area.val;


    MUINT32 LTM_Cnt;

    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_CTRL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.ctrl.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_BLK_NUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.blk_num.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_BLK_SZ))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.blk_sz.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_BLK_DIVX))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.blk_divx.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_BLK_DIVY))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.blk_divy.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_MAX_DIV))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.max_div.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_CLIP))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.clip.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_TILE_NUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.tile_num.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_TILE_CNTX))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.tile_cntx.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_TILE_CNTY))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.tile_cnty.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_CFG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.cfg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_RESET))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.reset.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_INTEN))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.inten.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_INTSTA))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.intsta.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_STATUS))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.status.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_INPUT_COUNT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.input_count.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_OUTPUT_COUNT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.output_count.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_CHKSUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.chksum.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_TILE_SIZE))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.tile_size.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_TILE_EDGE))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.tile_edge.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_TILE_CROP))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.tile_crop.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_DUMMY_REG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.dummy_reg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SRAM_CFG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.sram_cfg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SRAM_STATUS))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.sram_status.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_ATPG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.atpg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SHADOW_CTRL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.shadow_ctrl.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_GRAD0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_grad0.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_GRAD1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_grad1.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_GRAD2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_grad2.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_GRAD3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_grad3.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_TH0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_th0.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_TH1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_th1.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_TH2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_th2.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_TH3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_th3.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_SLP0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_slp0.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_SLP1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_slp1.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_SLP2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_slp2.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_SLP3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_slp3.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_SLP4))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_slp4.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_SLP5))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_slp5.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SELRGB_SLP6))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.selrgb_slp6.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_OUT_STR))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.out_str.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R1_LTM_SRAM_PINGPONG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R1.sram_pingpong.val;

#if 0
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_CTRL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.ctrl.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_BLK_NUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.blk_num.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_BLK_SZ))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.blk_sz.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_BLK_DIVX))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.blk_divx.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_BLK_DIVY))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.blk_divy.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_MAX_DIV))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.max_div.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_CLIP))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.clip.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_TILE_NUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.tile_num.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_TILE_CNTX))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.tile_cntx.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_TILE_CNTY))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.tile_cnty.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_CFG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.cfg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_RESET))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.reset.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_INTEN))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.inten.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_INTSTA))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.intsta.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_STATUS))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.status.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_INPUT_COUNT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.input_count.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_OUTPUT_COUNT))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.output_count.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_CHKSUM))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.chksum.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_TILE_SIZE))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.tile_size.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_TILE_EDGE))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.tile_edge.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_TILE_CROP))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.tile_crop.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_DUMMY_REG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.dummy_reg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SRAM_CFG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.sram_cfg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SRAM_STATUS))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.sram_status.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_ATPG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.atpg.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SHADOW_CTRL))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.shadow_ctrl.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_GRAD0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_grad0.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_GRAD1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_grad1.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_GRAD2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_grad2.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_GRAD3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_grad3.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_TH0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_th0.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_TH1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_th1.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_TH2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_th2.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_TH3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_th3.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_SLP0))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_slp0.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_SLP1))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_slp1.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_SLP2))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_slp2.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_SLP3))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_slp3.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_SLP4))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_slp4.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_SLP5))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_slp5.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SELRGB_SLP6))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.selrgb_slp6.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_OUT_STR))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.out_str.val;
    LTM_Cnt = ((MUINT32)offsetof(cam_reg_t, LTM_R2_LTM_SRAM_PINGPONG))/4;
    *(rDbgIspInfo.P1RegInfo.regData + LTM_Cnt) = LTM_R2.sram_pingpong.val;
#endif
*/
    return  MERR_OK;
}

