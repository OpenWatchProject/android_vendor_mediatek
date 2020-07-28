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
#include <aaa_hal_if.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <camera_feature.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include <pca_mgr.h>
#include <ccm_mgr.h>
#include "paramctrl.h"
#include <isp_tuning_buf.h>

using namespace android;
using namespace NSFeature;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSIspExifDebug;


#define _A_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

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
    rTags[eTagID].u4ID  = getIspTag(eTagID);
    rTags[eTagID].u4Val = i4Idx;
}

//  Tag <- Isp Regs.
template <class ISP_xxx_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags(
    ESensorDev_T eSensorDev,
    MBOOL fgReadFromHW,
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;

    //CAM_LOGD("fgReadFromHW = %d", fgReadFromHW);

    if (fgReadFromHW) {
        getIspReg(eSensorDev, param);
    }
    else {
        getIspHWBuf(eSensorDev, param);
    }

    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        //CAM_LOGD("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num,
        tag_index_over_total_num
    );
}

template <class ISP_xxx_T, typename ISP_MGR_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags2(
    ESensorDev_T eSensorDev,
    MBOOL /*fgReadFromHW*/,
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;

    //CAM_LOGD("fgReadFromHW = %d", fgReadFromHW);
    ISP_MGR_T::getInstance(eSensorDev).get(param);

    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        //CAM_LOGD("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num,
        tag_index_over_total_num
    );
}

template <class ISP_xxx_T, typename ISP_MGR_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags2(
    ESensorDev_T eSensorDev,
    const dip_x_reg_t* pReg,
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;

    ISP_MGR_T::getInstance(eSensorDev).get(param, pReg);

    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        //CAM_LOGD("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num,
        tag_index_over_total_num
    );
}

#if 0
//  Tag <- Isp Regs.
template <class ISP_xxx_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags(
    ESensorDev_T eSensorDev,
    ESensorTG_T eSensorTG,
    MBOOL fgReadFromHW,
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;

    if (fgReadFromHW) {
        getIspReg(eSensorDev, eSensorTG, param);
    }
    else {
        getIspHWBuf(eSensorDev, eSensorTG, param);
    }

    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        //CAM_LOGD("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num,
        tag_index_over_total_num
    );
}
#endif
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfoP1(NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, MBOOL const /*fgReadFromHW*/) const
{
    Mutex::Autolock lock(m_Lock);

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

    setIspIdx(rTags, M_P1_Profile,       m_rIspCamInfo.rMapping_Info.eIspProfile);
    setIspIdx(rTags, M_P1_SensorMode,    m_rIspCamInfo.rMapping_Info.eSensorMode);
    setIspIdx(rTags, M_P1_FrontalBin,    m_rIspCamInfo.rMapping_Info.eFrontBin);
    setIspIdx(rTags, M_P1_P2Size,        m_rIspCamInfo.rMapping_Info.eP2size);
    setIspIdx(rTags, M_P1_Flash,         m_rIspCamInfo.rMapping_Info.eFlash);
    setIspIdx(rTags, M_P1_APP,           m_rIspCamInfo.rMapping_Info.eApp);
    setIspIdx(rTags, M_P1_FaceDetection, m_rIspCamInfo.rMapping_Info.eFaceDetection);
    setIspIdx(rTags, M_P1_LensID,        m_rIspCamInfo.rMapping_Info.eLensID);
    setIspIdx(rTags, M_P1_DriverIC,      m_rIspCamInfo.rMapping_Info.eDriverIC);
    setIspIdx(rTags, M_P1_Custom,        m_rIspCamInfo.rMapping_Info.eCustom);
    setIspIdx(rTags, M_P1_ZoomIdx,       m_rIspCamInfo.rMapping_Info.eZoom_Idx);
    setIspIdx(rTags, M_P1_LV_Idx,        m_rIspCamInfo.rMapping_Info.eLV_Idx);
    setIspIdx(rTags, M_P1_CT_Idx,        m_rIspCamInfo.rMapping_Info.eCT_Idx);
    setIspIdx(rTags, M_P1_ISO_Idx,       m_rIspCamInfo.rMapping_Info.eISO_Idx);

    // RAWIspCamInfo
    setIspIdx(rTags, isHDR_P1,           m_rIspCamInfo.fgHDR);
    setIspIdx(rTags, SceneIdx_P1,        m_rIspCamInfo.eIdx_Scene);
    setIspIdx(rTags, UPPER_ISO_IDX_P1,   m_rIspCamInfo.eIdx_ISO_U);
    setIspIdx(rTags, LOWER_ISO_IDX_P1,   m_rIspCamInfo.eIdx_ISO_L);
    setIspIdx(rTags, UPPER_ZOOM_IDX_P1,  m_rIspCamInfo.eIdx_Zoom_U);
    setIspIdx(rTags, LOWER_ZOOM_IDX_P1,  m_rIspCamInfo.eIdx_Zoom_L);
    setIspIdx(rTags, UPPER_CT_IDX_P1,    m_rIspCamInfo.eIdx_CT_U);
    setIspIdx(rTags, LOWER_CT_IDX_P1,    m_rIspCamInfo.eIdx_CT_L);
    setIspIdx(rTags, ZoomRatio_x100_P1,  m_rIspCamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, ShadingIdx_P1,      m_rIspCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, Custom_Interp_P1, m_pIspTuningCustom->is_to_invoke_isp_interpolation(m_rIspCamInfo));


    //Only P1 paramctrl info
    setIspIdx(rTags, SwnrEncEnableIsoThreshold, m_u4SwnrEncEnableIsoThreshold);
    setIspIdx(rTags, Dbg_Interp, m_IspInterpCtrl);
    setIspIdx(rTags, RawFmt, m_u4RawFmtType);
    setIspIdx(rTags, SensorDev, m_eSensorDev);

    //AE Info
    setIspIdx(rTags, AE_INFO_EXP_TIME_P1, m_rIspCamInfo.rAEInfo.u4Eposuretime);
    setIspIdx(rTags, AE_INFO_AFE_GAIN_P1, m_rIspCamInfo.rAEInfo.u4AfeGain);
    setIspIdx(rTags, AE_INFO_ISP_GAIN_P1, m_rIspCamInfo.rAEInfo.u4IspGain);
    setIspIdx(rTags, AE_INFO_REAL_ISO_P1, m_rIspCamInfo.rAEInfo.u4RealISOValue);
    setIspIdx(rTags, AE_INFO_LV_x10_P1, m_rIspCamInfo.rAEInfo.i4RealLightValue_x10);

    MUINT32 dbg_OBC_AEGain;
    ISP_MGR_OBC_T::getInstance(m_eSensorDev).getIspAEGain(&dbg_OBC_AEGain);
    setIspIdx(rTags, OBC_AE_GAIN, dbg_OBC_AEGain);

    MINT32 dbg_RPG_FlareGain, dbg_RPG_FlareOfst;
    ISP_MGR_RPG_T::getInstance(m_eSensorDev).getIspFlare(&dbg_RPG_FlareGain, &dbg_RPG_FlareOfst);
    setIspIdx(rTags, RPG_FLARE_GAIN, dbg_RPG_FlareGain);
    setIspIdx(rTags, RPG_FLARE_OFST, dbg_RPG_FlareOfst);

    //AWB Info
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_R_P1, m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4R);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_G_P1, m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4G);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_B_P1, m_rIspCamInfo.rAWBInfo.rCurrentAWBGain.i4B);
    setIspIdx(rTags, AWB_INFO_CT_P1, m_rIspCamInfo.rAWBInfo.i4CCT);

    ////////////////////////////////////////////////////////////////////////////
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////

    // CAM_CTL_EN_P1
    CAM_LOGD_IF(m_bDebugEnable,"CAM_CTL_EN_P1:");

    // SL2F
    CAM_LOGD_IF(m_bDebugEnable,"SL2F:");
    setIspIdx(rTags, IDX_SL2F, m_ParamIdx_P1.SL2F);

    // DBS
    CAM_LOGD_IF(m_bDebugEnable,"DBS:");
    setIspIdx(rTags, IDX_DBS, m_ParamIdx_P1.DBS);
    setIspIdx(rTags, IDX_DBS_UPPER, m_ISP_INT.sDbs.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_DBS_LOWER, m_ISP_INT.sDbs.u2LowerIso_LowerZoom);

    // ADBS
    setIspIdx(rTags, IDX_ADBS, m_ParamIdx_P1.ADBS);
    setIspIdx(rTags, IDX_ADBS_UPPER, m_ISP_INT.sAdbs.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_ADBS_LOWER, m_ISP_INT.sAdbs.u2LowerIso_LowerZoom);

    // OBC
    CAM_LOGD_IF(m_bDebugEnable,"OBC:");
    setIspIdx(rTags, IDX_OBC, m_ParamIdx_P1.OBC);
    setIspIdx(rTags, IDX_OBC_UPPER, m_ISP_INT.sObc.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_OBC_LOWER, m_ISP_INT.sObc.u2LowerIso_LowerZoom);

    // BPC
    CAM_LOGD_IF(m_bDebugEnable,"BPC:");
    setIspIdx(rTags, IDX_BPC, m_ParamIdx_P1.BNR_BPC);
    setIspIdx(rTags, IDX_BPC_UPPER, m_ISP_INT.sBpc.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_BPC_LOWER, m_ISP_INT.sBpc.u2LowerIso_LowerZoom);

    // NR1
    CAM_LOGD_IF(m_bDebugEnable,"NR1:");
    setIspIdx(rTags, IDX_NR1, m_ParamIdx_P1.BNR_NR1);
    setIspIdx(rTags, IDX_NR1_UPPER, m_ISP_INT.sNr1.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_NR1_LOWER, m_ISP_INT.sNr1.u2LowerIso_LowerZoom);

    // PDC
    CAM_LOGD_IF(m_bDebugEnable,"PDC:");
    setIspIdx(rTags, IDX_PDC, m_ParamIdx_P1.BNR_PDC);
    setIspIdx(rTags, IDX_PDC_UPPER, m_ISP_INT.sPdc.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_PDC_LOWER, m_ISP_INT.sPdc.u2LowerIso_LowerZoom);

    // RMM
    CAM_LOGD_IF(m_bDebugEnable,"RMM:");
    setIspIdx(rTags, IDX_RMM, m_ParamIdx_P1.RMM);
    setIspIdx(rTags, IDX_RMM_UPPER, m_ISP_INT.sRmm.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_RMM_LOWER, m_ISP_INT.sRmm.u2LowerIso_LowerZoom);
    MUINT32 dbg_RmmIdx = ISP_MGR_RMM_T::getInstance(m_eSensorDev).getRMM_SWIndex();
    setIspIdx(rTags, RMM_SW_IDX, dbg_RmmIdx);

    // RMG
    CAM_LOGD_IF(m_bDebugEnable,"RMG:");

    // LSC
    CAM_LOGD_IF(m_bDebugEnable,"LSC:");

    // RPG
    CAM_LOGD_IF(m_bDebugEnable,"RPG:");

    // LCS
    MY_LOG_IF(m_bDebugEnable,"LCS:");

    MUINT32 u4RegCnt = (_A_P1(CAM_SL2F_SIZE) / 4) + 1;
    MUINT32 u4RRZOfst = (_A_P1(CAM_RRZ_CTL) / 4); //start address of rrz register
    MVOID* pP1TuningBuffer = IspTuningBufCtrl::getInstance(m_eSensorDev)->getP1Buffer();

    if(u4RegCnt > P1_BUFFER_SIZE)
    {
        CAM_LOGE("P1 buffer:(%d) byte exceed EXIF allocation size:(%d) byte", u4RegCnt*4, P1_BUFFER_SIZE*4);
    }
    else
    {
        if (pP1TuningBuffer != NULL)
        {
            memcpy(rDebugInfo.P1RegInfo.regData, pP1TuningBuffer, P1_BUFFER_SIZE*4);
            memcpy(rDebugInfo.P1RegInfo.regData+u4RRZOfst, &m_RRZInfo, sizeof(ISP_NVRAM_RRZ_T)); //overwrite rrz register by member
            rDebugInfo.P1RegInfo.u4TableSize = P1_BUFFER_SIZE+1;
            rDebugInfo.P1RegInfo.u4HwVersion = 5;
        }
    }

    // CCM P1
    setIspIdx(rTags, DYNAMIC_CCM_IDX_P1, m_ISP_DYM.sCcm_tbl_P1_Idx);

    // CCU
    setIspIdx(rTags, CCUEnable,   0);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfo(const NSIspTuning::ISP_INFO_T& rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, const void* pRegBuf, const void* pMfbBuf ) const
{
    Mutex::Autolock lock(m_Lock);
    const MBOOL fgReadFromHW = MFALSE;
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
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////

    // CAM_CTL_EN_P2
    CAM_LOGD_IF(m_bDebugEnable,"CAM_CTL_EN_P2:");

    // SL2G
    CAM_LOGD_IF(m_bDebugEnable,"SL2G:");
    setIspIdx(rTags, IDX_SL2G, m_ParamIdx_P2.SL2F);

    // DBS2
    CAM_LOGD_IF(m_bDebugEnable,"DBS2:");
    setIspIdx(rTags, IDX_DBS2, m_ParamIdx_P2.DBS);
    setIspIdx(rTags, IDX_DBS2_UPPER, m_ISP_INT.sDbs2.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_DBS2_LOWER, m_ISP_INT.sDbs2.u2LowerIso_LowerZoom);

    // ADBS2
    CAM_LOGD_IF(m_bDebugEnable,"ADBS2:");
    setIspIdx(rTags, IDX_ADBS2, m_ParamIdx_P2.ADBS);
    setIspIdx(rTags, IDX_ADBS2_UPPER, m_ISP_INT.sAdbs2.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_ADBS2_LOWER, m_ISP_INT.sAdbs2.u2LowerIso_LowerZoom);

    // OBC2
    CAM_LOGD_IF(m_bDebugEnable,"OBC2:");
    setIspIdx(rTags, IDX_OBC2, m_ParamIdx_P2.OBC);
    setIspIdx(rTags, IDX_OBC2_UPPER, m_ISP_INT.sObc2.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_OBC2_LOWER, m_ISP_INT.sObc2.u2LowerIso_LowerZoom);

    // BPC2
    CAM_LOGD_IF(m_bDebugEnable,"BPC2:");
    setIspIdx(rTags, IDX_BPC2, m_ParamIdx_P2.BNR_BPC);
    setIspIdx(rTags, IDX_BPC2_UPPER, m_ISP_INT.sBpc2.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_BPC2_LOWER, m_ISP_INT.sBpc2.u2LowerIso_LowerZoom);

    // NR12
    CAM_LOGD_IF(m_bDebugEnable,"NR12:");
    setIspIdx(rTags, IDX_NR12, m_ParamIdx_P2.BNR_NR1);
    setIspIdx(rTags, IDX_NR12_UPPER, m_ISP_INT.sNr12.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_NR12_LOWER, m_ISP_INT.sNr12.u2LowerIso_LowerZoom);

    // PDC2
    CAM_LOGD_IF(m_bDebugEnable,"PDC2:");
    setIspIdx(rTags, IDX_PDC2, m_ParamIdx_P2.BNR_PDC);
    setIspIdx(rTags, IDX_PDC2_UPPER, m_ISP_INT.sPdc2.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_PDC2_LOWER, m_ISP_INT.sPdc2.u2LowerIso_LowerZoom);
/*
    // RMM2
    CAM_LOGD_IF(m_bDebugEnable,"RMM2:");
    setIspIdx(rTags, IDX_RMM2, m_ParamIdx_P2.RMM);
    setIspIdx(rTags, IDX_RMM2_UPPER, m_ISP_INT.sRmm2.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_RMM2_LOWER, m_ISP_INT.sRmm2.u2LowerIso_LowerZoom);
    MUINT32 dbg_Rmm2Idx = ISP_MGR_RMM2_T::getInstance(m_eSensorDev).getRMM2_SWIndex();
    setIspIdx(rTags, RMM2_SW_IDX, dbg_Rmm2Idx);

    // RMG2
    CAM_LOGD_IF(m_bDebugEnable,"RMG2:");
*/
    // LSC2
    CAM_LOGD_IF(m_bDebugEnable,"LSC2:");

    // PGN
    CAM_LOGD_IF(m_bDebugEnable,"PGN:");

    // SL2
    CAM_LOGD_IF(m_bDebugEnable,"SL2:");
    setIspIdx(rTags, IDX_SL2, m_ParamIdx_P2.SL2);

    // RNR
    CAM_LOGD_IF(m_bDebugEnable,"RNR:");
    setIspIdx(rTags, IDX_RNR, m_ParamIdx_P2.RNR);
    setIspIdx(rTags, IDX_RNR_UPPER, m_ISP_INT.sRnr.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_RNR_LOWER, m_ISP_INT.sRnr.u2LowerIso_LowerZoom);

    // UDM
    CAM_LOGD_IF(m_bDebugEnable,"UDM:");
    setIspIdx(rTags, IDX_UDM, m_ParamIdx_P2.UDM);
    setIspIdx(rTags, IDX_UDM_UPPER_ISO_UPPER_ZOOM, m_ISP_INT.sUdm.u2UpperIso_UpperZoom);
    setIspIdx(rTags, IDX_UDM_LOWER_ISO_UPPER_ZOOM, m_ISP_INT.sUdm.u2LowerIso_UpperZoom);
    setIspIdx(rTags, IDX_UDM_UPPER_ISO_LOWER_ZOOM, m_ISP_INT.sUdm.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_UDM_LOWER_ISO_LOWER_ZOOM, m_ISP_INT.sUdm.u2LowerIso_LowerZoom);
/*
    // G2G_CFC
    MY_LOG_IF(m_bDebugEnable,"G2G_CFC:");
    setIspIdx(rTags, IDX_CCM_CFC, idxmgr.getIdx_CCM_CFC());
    setIspTags2<ISP_NVRAM_CCM_CTL_T, ISP_MGR_CCM_T, DIP_X_G2G_CFC_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
*/
    // G2C
    CAM_LOGD_IF(m_bDebugEnable,"G2C:");

    // ANR
    CAM_LOGD_IF(m_bDebugEnable,"ANR:");
    setIspIdx(rTags, IDX_ANR, m_ParamIdx_P2.ANR);
    setIspIdx(rTags, IDX_ANR_UPPER_ISO_UPPER_ZOOM, m_ISP_INT.sAnr.u2UpperIso_UpperZoom);
    setIspIdx(rTags, IDX_ANR_LOWER_ISO_UPPER_ZOOM, m_ISP_INT.sAnr.u2LowerIso_UpperZoom);
    setIspIdx(rTags, IDX_ANR_UPPER_ISO_LOWER_ZOOM, m_ISP_INT.sAnr.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_ANR_LOWER_ISO_LOWER_ZOOM, m_ISP_INT.sAnr.u2LowerIso_LowerZoom);


    // ANR2
    CAM_LOGD_IF(m_bDebugEnable,"ANR2:");
    setIspIdx(rTags, IDX_ANR2, m_ParamIdx_P2.ANR2);
    setIspIdx(rTags, IDX_ANR2_UPPER_ISO_UPPER_ZOOM, m_ISP_INT.sAnr2.u2UpperIso_UpperZoom);
    setIspIdx(rTags, IDX_ANR2_LOWER_ISO_UPPER_ZOOM, m_ISP_INT.sAnr2.u2LowerIso_UpperZoom);
    setIspIdx(rTags, IDX_ANR2_UPPER_ISO_LOWER_ZOOM, m_ISP_INT.sAnr2.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_ANR2_LOWER_ISO_LOWER_ZOOM, m_ISP_INT.sAnr2.u2LowerIso_LowerZoom);

    // CCR
    CAM_LOGD_IF(m_bDebugEnable,"CCR:");
    setIspIdx(rTags, IDX_CCR, m_ParamIdx_P2.CCR);
    setIspIdx(rTags, IDX_CCR_UPPER, m_ISP_INT.sCcr.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_CCR_LOWER, m_ISP_INT.sCcr.u2LowerIso_LowerZoom);

    // ABF
    CAM_LOGD_IF(m_bDebugEnable,"ABF:");
    setIspIdx(rTags, IDX_ABF, m_ParamIdx_P2.ABF);
    setIspIdx(rTags, IDX_ABF_UPPER, m_ISP_INT.sAbf.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_ABF_LOWER, m_ISP_INT.sAbf.u2LowerIso_LowerZoom);

    // BOK
    CAM_LOGD_IF(m_bDebugEnable,"BOK:");
    setIspIdx(rTags, IDX_BOK, m_ParamIdx_P2.BOK);

    // HFG
    CAM_LOGD_IF(m_bDebugEnable,"HFG:");
    setIspIdx(rTags, IDX_HFG, m_ParamIdx_P2.HFG);
    setIspIdx(rTags, IDX_HFG_UPPER_ISO_UPPER_ZOOM, m_ISP_INT.sHfg.u2UpperIso_UpperZoom);
    setIspIdx(rTags, IDX_HFG_LOWER_ISO_UPPER_ZOOM, m_ISP_INT.sHfg.u2LowerIso_UpperZoom);
    setIspIdx(rTags, IDX_HFG_UPPER_ISO_LOWER_ZOOM, m_ISP_INT.sHfg.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_HFG_LOWER_ISO_LOWER_ZOOM, m_ISP_INT.sHfg.u2LowerIso_LowerZoom);

    //  EE
    CAM_LOGD_IF(m_bDebugEnable,"EE:");
    setIspIdx(rTags, IDX_EE, m_ParamIdx_P2.EE);
    setIspIdx(rTags, IDX_EE_UPPER_ISO_UPPER_ZOOM, m_ISP_INT.sEe.u2UpperIso_UpperZoom);
    setIspIdx(rTags, IDX_EE_LOWER_ISO_UPPER_ZOOM, m_ISP_INT.sEe.u2LowerIso_UpperZoom);
    setIspIdx(rTags, IDX_EE_UPPER_ISO_LOWER_ZOOM, m_ISP_INT.sEe.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_EE_LOWER_ISO_LOWER_ZOOM, m_ISP_INT.sEe.u2LowerIso_LowerZoom);


    // SE
    CAM_LOGD_IF(m_bDebugEnable,"SE:");

    // NR3D
    CAM_LOGD_IF(m_bDebugEnable,"NR3D:");
    setIspIdx(rTags, IDX_NR3D, m_ParamIdx_P2.NR3D);
    setIspIdx(rTags, IDX_NR3D_UPPER_ISO_UPPER_ZOOM, m_ISP_INT.sNr3d.u2UpperIso_UpperZoom);
    setIspIdx(rTags, IDX_NR3D_LOWER_ISO_UPPER_ZOOM, m_ISP_INT.sNr3d.u2LowerIso_UpperZoom);
    setIspIdx(rTags, IDX_NR3D_UPPER_ISO_LOWER_ZOOM, m_ISP_INT.sNr3d.u2UpperIso_LowerZoom);
    setIspIdx(rTags, IDX_NR3D_LOWER_ISO_LOWER_ZOOM, m_ISP_INT.sNr3d.u2LowerIso_LowerZoom);

    // MFB
    CAM_LOGD_IF(m_bDebugEnable,"MFB:");
    setIspIdx(rTags, IDX_MFB, m_ParamIdx_P2.MFB);

    // MIXER3
    CAM_LOGD_IF(m_bDebugEnable,"MIXER3:");
    setIspIdx(rTags, IDX_MIX3, m_ParamIdx_P2.MIXER3);

    // LCE
    CAM_LOGD_IF(m_bDebugEnable,"LCE:");

    // PCA
    CAM_LOGD_IF(m_bDebugEnable,"PCA:");

    // G2G
    CAM_LOGD_IF(m_bDebugEnable,"G2G:");
    setIspIdx(rTags, DYNAMIC_CCM_IDX_P1, m_ISP_DYM.sCcm_tbl_P1_Idx);
    setIspIdx(rTags, DYNAMIC_CCM_IDX_P2, m_ISP_DYM.sCcm_tbl_P2_Idx);
    setIspIdx(rTags, SMOOTH_COLOR_Ver, m_ISP_ALG_VER.sCOLOR);
    setIspIdx(rTags, SMOOTH_COLOR_UPPER_LV_UPPER_CT, m_ISP_INT.sColor.u2UpperLv_UpperCt);
    setIspIdx(rTags, SMOOTH_COLOR_LOWER_LV_UPPER_CT, m_ISP_INT.sColor.u2LowerLv_UpperCt);
    setIspIdx(rTags, SMOOTH_COLOR_UPPER_LV_LOWER_CT, m_ISP_INT.sColor.u2UpperLv_LowerCt);
    setIspIdx(rTags, SMOOTH_COLOR_LOWER_LV_LOWER_CT, m_ISP_INT.sColor.u2LowerLv_LowerCt);
    setIspIdx(rTags, SMOOTH_COLOR_PARAM_IDX        , m_ISP_INT.sColor.u2ParamIdx);

    setIspIdx(rTags, SMOOTH_CCM_UPPER_LV        , m_ISP_INT.sCCM2.u2UpperLv_LowerCt);
    setIspIdx(rTags, SMOOTH_CCM_LOWER_LV        , m_ISP_INT.sCCM2.u2LowerLv_LowerCt);

    setIspIdx(rTags, SMOOTH_FD_ANR_UPPER_LV        , m_ISP_INT.sFD_ANR.u2UpperLv_LowerCt);
    setIspIdx(rTags, SMOOTH_FD_ANR_LOWER_LV        , m_ISP_INT.sFD_ANR.u2LowerLv_UpperCt);
    setIspIdx(rTags, SMOOTH_ANR_TBL_UPPER_LV        , m_ISP_INT.NBC_TBL.u2UpperLv_LowerCt);
    setIspIdx(rTags, SMOOTH_ANR_TBL_LOWER_LV        , m_ISP_INT.NBC_TBL.u2LowerLv_UpperCt);
    setIspIdx(rTags, SMOOTH_CCM_Ver, m_ISP_ALG_VER.sCCM);

    //LCS Info
    setIspIdx(rTags, LCS_IN_Width, rCamInfo.rLCS_Info.rOutSetting.u4InWidth);
    setIspIdx(rTags, LCS_IN_Height, rCamInfo.rLCS_Info.rOutSetting.u4InHeight);
    setIspIdx(rTags, LCS_OUT_Width, rCamInfo.rLCS_Info.rOutSetting.u4OutWidth);
    setIspIdx(rTags, LCS_OUT_Height, rCamInfo.rLCS_Info.rOutSetting.u4OutHeight);
    setIspIdx(rTags, LCS_CROP_X, rCamInfo.rLCS_Info.rOutSetting.u4CropX);
    setIspIdx(rTags, LCS_CROP_Y, rCamInfo.rLCS_Info.rOutSetting.u4CropY);
    setIspIdx(rTags, LCS_LRZR_1, rCamInfo.rLCS_Info.rOutSetting.u4Lrzr1);
    setIspIdx(rTags, LCS_LRZR_2, rCamInfo.rLCS_Info.rOutSetting.u4Lrzr2);
    setIspIdx(rTags, FrontBin_ByLCS, rCamInfo.rLCS_Info.rOutSetting.bFrontBin);
    setIspIdx(rTags, QBin_Rto_ByLCS, rCamInfo.rLCS_Info.rOutSetting.u4Qbin_Rto);

    //Size Info
    setIspIdx(rTags, TG_Size_W,       rCamInfo.rCropRzInfo.i4TGoutW);
    setIspIdx(rTags, TG_Size_H,       rCamInfo.rCropRzInfo.i4TGoutH);
    setIspIdx(rTags, FBinOnOff,       rCamInfo.rCropRzInfo.fgFBinOnOff);
    setIspIdx(rTags, RRZOnOff,        rCamInfo.rCropRzInfo.fgRRZOnOff);
    setIspIdx(rTags, RRZ_In_Size_W,   rCamInfo.rCropRzInfo.i4RRZinW);
    setIspIdx(rTags, RRZ_In_Size_H,   rCamInfo.rCropRzInfo.i4RRZinH);
    setIspIdx(rTags, RRZ_Crop_Ofst_X, rCamInfo.rCropRzInfo.i4RRZofstX);
    setIspIdx(rTags, RRZ_Crop_Ofst_Y, rCamInfo.rCropRzInfo.i4RRZofstY);
    setIspIdx(rTags, RRZ_Crop_Size_W, rCamInfo.rCropRzInfo.i4RRZcropW);
    setIspIdx(rTags, RRZ_Crop_Size_H, rCamInfo.rCropRzInfo.i4RRZcropH);
    setIspIdx(rTags, RRZ_Out_W,       rCamInfo.rCropRzInfo.i4RRZoutW);
    setIspIdx(rTags, RRZ_Out_H,       rCamInfo.rCropRzInfo.i4RRZoutH);

    setIspIdx(rTags, M_P2_Profile,       rCamInfo.rMapping_Info.eIspProfile);
    setIspIdx(rTags, M_P2_SensorMode,    rCamInfo.rMapping_Info.eSensorMode);
    setIspIdx(rTags, M_P2_FrontalBin,    rCamInfo.rMapping_Info.eFrontBin);
    setIspIdx(rTags, M_P2_P2Size,        rCamInfo.rMapping_Info.eP2size);
    setIspIdx(rTags, M_P2_Flash,         rCamInfo.rMapping_Info.eFlash);
    setIspIdx(rTags, M_P2_APP,           rCamInfo.rMapping_Info.eApp);
    setIspIdx(rTags, M_P2_FaceDetection, rCamInfo.rMapping_Info.eFaceDetection);
    setIspIdx(rTags, M_P2_LensID,        rCamInfo.rMapping_Info.eLensID);
    setIspIdx(rTags, M_P2_DriverIC,      rCamInfo.rMapping_Info.eDriverIC);
    setIspIdx(rTags, M_P2_Custom,        rCamInfo.rMapping_Info.eCustom);
    setIspIdx(rTags, M_P2_ZoomIdx,       rCamInfo.rMapping_Info.eZoom_Idx);
    setIspIdx(rTags, M_P2_LV_Idx,        rCamInfo.rMapping_Info.eLV_Idx);
    setIspIdx(rTags, M_P2_CT_Idx,        rCamInfo.rMapping_Info.eCT_Idx);
    setIspIdx(rTags, M_P2_ISO_Idx,       rCamInfo.rMapping_Info.eISO_Idx);

    //SW_ANR_LTM_PARAM
    ISP_NBC_LTM_SW_PARAM_T SwAnrParam;
    MUINT16 SW_ANR_LTM_Ver;
    ISP_MGR_NBC_T::getInstance(m_eSensorDev).getSwParam(SwAnrParam, SW_ANR_LTM_Ver);
    setIspIdx(rTags, FW_NBC_IDX_UPPER_ISO, m_ISP_INT.sSwAnrLtm.u2UpperIso_LowerZoom);
    setIspIdx(rTags, FW_NBC_IDX_LOWER_ISO, m_ISP_INT.sSwAnrLtm.u2LowerIso_LowerZoom);
    setIspIdx(rTags, FW_NBC_VER, SW_ANR_LTM_Ver);
    setIspIdx(rTags, FW_NBC_TC_LINK_STR, SwAnrParam.TC_LINK_STR);
    setIspIdx(rTags, FW_NBC_CE_LINK_STR, SwAnrParam.CE_LINK_STR);
    setIspIdx(rTags, FW_NBC_GAIN_CLIP_HI,SwAnrParam.GAIN_CLIP_HI);
    setIspIdx(rTags, FW_NBC_GAIN_CLIP_LO, SwAnrParam.GAIN_CLIP_LO);
    setIspIdx(rTags, FW_NBC_GAMMA_SMOOTH, SwAnrParam.GAMMA_SMOOTH);
    setIspIdx(rTags, FW_NBC_P50_GAIN, SwAnrParam.P50_GAIN);
    setIspIdx(rTags, FW_NBC_P250_GAIN, SwAnrParam.P250_GAIN);
    setIspIdx(rTags, FW_NBC_P500_GAIN, SwAnrParam.P500_GAIN);
    setIspIdx(rTags, FW_NBC_P750_GAIN, SwAnrParam.P750_GAIN);
    setIspIdx(rTags, FW_NBC_P950_GAIN, SwAnrParam.P950_GAIN);
    setIspIdx(rTags, FW_NBC_RSV1, SwAnrParam.RSV1);
    setIspIdx(rTags, FW_NBC_RSV2, SwAnrParam.RSV2);
    setIspIdx(rTags, FW_NBC_RSV3, SwAnrParam.RSV3);
    setIspIdx(rTags, FW_NBC_RSV4, SwAnrParam.RSV4);
    setIspIdx(rTags, FW_NBC_RSV5, SwAnrParam.RSV5);
    setIspIdx(rTags, FW_NBC_RSV6, SwAnrParam.RSV6);
    setIspIdx(rTags, FW_NBC_RSV7, SwAnrParam.RSV7);
    setIspIdx(rTags, FW_NBC_RSV8, SwAnrParam.RSV8);

    setIspIdx(rTags, ISO_THRESHOLD_ENABLE, m_bUseIsoThreshEnable);
    setIspIdx(rTags, ISO_THRESHOLD, m_isoThresh);
    setIspIdx(rTags, ISO_THRESHOLD_PREVIOUS_ISO, m_i4PreRealISO);

    //Metadata
    setIspIdx(rTags, Meta_PureRaw,    (rCamInfo.i4RawType == NSIspTuning::ERawType_Pure) ? 1 : 0);
    setIspIdx(rTags, Meta_PGNEnable,   !(rCamInfo.fgRPGEnable));
    setIspIdx(rTags, Meta_ByPassLCE,   rCamInfo.bBypassLCE);
    // LCEBuffer
    setIspIdx(rTags, LCEBuffer,       (m_pLCSBuffer) ? 1 : 0);

    //dynamic Gamma
    // adaptive Gamma
    CAM_LOGD_IF(m_bDebugEnable,"adaptive Gamma:");
    setIspIdx(rTags, DIP_X_GMA_GMAMode, m_GmaExifInfo.i4GMAMode);
    setIspIdx(rTags, DIP_X_GMA_FaceGMAFlag, m_GmaExifInfo.i4FaceGMAFlag);
    setIspIdx(rTags, DIP_X_GMA_GmaProfile, m_GmaExifInfo.i4GmaProfile);
    setIspIdx(rTags, DIP_X_GMA_ChipVersion, m_GmaExifInfo.i4ChipVersion);
    setIspIdx(rTags, DIP_X_GMA_MainVersion, m_GmaExifInfo.i4MainVersion);
    setIspIdx(rTags, DIP_X_GMA_SubVersion, m_GmaExifInfo.i4SubVersion);
    setIspIdx(rTags, DIP_X_GMA_SystemVersion, m_GmaExifInfo.i4SystemVersion);
    setIspIdx(rTags, DIP_X_GMA_EVRatio, m_GmaExifInfo.i4EVRatio);
    setIspIdx(rTags, DIP_X_GMA_LowContrastThr, m_GmaExifInfo.i4LowContrastThr);
    setIspIdx(rTags, DIP_X_GMA_LowContrastRatio, m_GmaExifInfo.i4LowContrastRatio);
    setIspIdx(rTags, DIP_X_GMA_LowContrastSeg, m_GmaExifInfo.i4LowContrastSeg);
    setIspIdx(rTags, DIP_X_GMA_Contrast, m_GmaExifInfo.i4Contrast);
    setIspIdx(rTags, DIP_X_GMA_Contrast_L, m_GmaExifInfo.i4Contrast_L);
    setIspIdx(rTags, DIP_X_GMA_Contrast_H, m_GmaExifInfo.i4Contrast_H);
    setIspIdx(rTags, DIP_X_GMA_HdrContrastWeight, m_GmaExifInfo.i4HdrContrastWeight);
    setIspIdx(rTags, DIP_X_GMA_EVContrastY, m_GmaExifInfo.i4EVContrastY);
    setIspIdx(rTags, DIP_X_GMA_ContrastY_L, m_GmaExifInfo.i4ContrastY_L);
    setIspIdx(rTags, DIP_X_GMA_ContrastY_H, m_GmaExifInfo.i4ContrastY_H);
    setIspIdx(rTags, DIP_X_GMA_NightContrastWeight, m_GmaExifInfo.i4NightContrastWeight);
    setIspIdx(rTags, DIP_X_GMA_LV, m_GmaExifInfo.i4LV);
    setIspIdx(rTags, DIP_X_GMA_LV_L, m_GmaExifInfo.i4LV_L);
    setIspIdx(rTags, DIP_X_GMA_LV_H, m_GmaExifInfo.i4LV_H);
    setIspIdx(rTags, DIP_X_GMA_HdrLVWeight, m_GmaExifInfo.i4HdrLVWeight);
    setIspIdx(rTags, DIP_X_GMA_NightLVWeight, m_GmaExifInfo.i4NightLVWeight);
    setIspIdx(rTags, DIP_X_GMA_SmoothEnable, m_GmaExifInfo.i4SmoothEnable);
    setIspIdx(rTags, DIP_X_GMA_SmoothSpeed, m_GmaExifInfo.i4SmoothSpeed);
    setIspIdx(rTags, DIP_X_GMA_SmoothWaitAE, m_GmaExifInfo.i4SmoothWaitAE);
    setIspIdx(rTags, DIP_X_GMA_GMACurveEnable, m_GmaExifInfo.i4GMACurveEnable);
    setIspIdx(rTags, DIP_X_GMA_CenterPt, m_GmaExifInfo.i4CenterPt);
    setIspIdx(rTags, DIP_X_GMA_LowCurve, m_GmaExifInfo.i4LowCurve);
    setIspIdx(rTags, DIP_X_GMA_SlopeL, m_GmaExifInfo.i4SlopeL);
    setIspIdx(rTags, DIP_X_GMA_FlareEnable, m_GmaExifInfo.i4FlareEnable);
    setIspIdx(rTags, DIP_X_GMA_FlareOffset, m_GmaExifInfo.i4FlareOffset);
    setIspIdx(rTags, DIP_X_GMA_IDX, (MUINT32)m_pGmaMgr->getIDX());

    //dynamic LCE
    // adaptive LCE
    CAM_LOGD_IF(m_bDebugEnable,"adaptive LCE:");
    setIspIdx(rTags,  DIP_X_LCE_ChipVersion      ,  m_LceExifInfo.i4ChipVersion);
    setIspIdx(rTags,  DIP_X_LCE_MainVersion      ,  m_LceExifInfo.i4MainVersion);
    setIspIdx(rTags,  DIP_X_LCE_SubVersion       ,  m_LceExifInfo.i4SubVersion);
    setIspIdx(rTags,  DIP_X_LCE_SystemVersion    ,  m_LceExifInfo.i4SystemVersion);
    setIspIdx(rTags,  DIP_X_LCE_LV               ,  m_LceExifInfo.i4LV);
    setIspIdx(rTags,  DIP_X_LCE_LVIdx_L          ,  m_LceExifInfo.i4LVIdx_L);
    setIspIdx(rTags,  DIP_X_LCE_LVIdx_H          ,  m_LceExifInfo.i4LVIdx_H);
    setIspIdx(rTags,  DIP_X_LCE_DiffRangeIdx_L   ,  m_LceExifInfo.i4DiffRangeIdx_L);
    setIspIdx(rTags,  DIP_X_LCE_DiffRangeIdx_H   ,  m_LceExifInfo.i4DiffRangeIdx_H);
    setIspIdx(rTags,  DIP_X_LCE_LCEScene         ,  m_LceExifInfo.i4LCEScene);
    setIspIdx(rTags,  DIP_X_LCE_LVTarget         ,  m_LceExifInfo.i4LVTarget);
    setIspIdx(rTags,  DIP_X_LCE_LumaTarget       ,  m_LceExifInfo.i4LumaTarget);
    setIspIdx(rTags,  DIP_X_LCE_BrightAvg        ,  m_LceExifInfo.i4BrightAvg);
    setIspIdx(rTags,  DIP_X_LCE_DarkLumaProb     ,  m_LceExifInfo.i4DarkLumaProb);
    setIspIdx(rTags,  DIP_X_LCE_NewLumaTarget    ,  m_LceExifInfo.i4NewLumaTarget);
    setIspIdx(rTags,  DIP_X_LCE_LumaProb         ,  m_LceExifInfo.i4LumaProb);
    setIspIdx(rTags,  DIP_X_LCE_FlatTarget       ,  m_LceExifInfo.i4FlatTarget);
    setIspIdx(rTags,  DIP_X_LCE_FlatProb         ,  m_LceExifInfo.i4FlatProb);
    setIspIdx(rTags,  DIP_X_LCE_FinalTarget      ,  m_LceExifInfo.i4FinalTarget);
    setIspIdx(rTags,  DIP_X_LCE_FinalDStrength   ,  m_LceExifInfo.i4FinalDStrength);
    setIspIdx(rTags,  DIP_X_LCE_FinalBStrength   ,  m_LceExifInfo.i4FinalBStrength);
    setIspIdx(rTags,  DIP_X_LCE_P0               ,  m_LceExifInfo.i4P0);
    setIspIdx(rTags,  DIP_X_LCE_P1               ,  m_LceExifInfo.i4P1);
    setIspIdx(rTags,  DIP_X_LCE_P50              ,  m_LceExifInfo.i4P50);
    setIspIdx(rTags,  DIP_X_LCE_P250             ,  m_LceExifInfo.i4P250);
    setIspIdx(rTags,  DIP_X_LCE_P500             ,  m_LceExifInfo.i4P500);
    setIspIdx(rTags,  DIP_X_LCE_P750             ,  m_LceExifInfo.i4P750);
    setIspIdx(rTags,  DIP_X_LCE_P950             ,  m_LceExifInfo.i4P950);
    setIspIdx(rTags,  DIP_X_LCE_P999             ,  m_LceExifInfo.i4P999);
    setIspIdx(rTags,  DIP_X_LCE_O0               ,  m_LceExifInfo.i4O0);
    setIspIdx(rTags,  DIP_X_LCE_O1               ,  m_LceExifInfo.i4O1);
    setIspIdx(rTags,  DIP_X_LCE_O50              ,  m_LceExifInfo.i4O50);
    setIspIdx(rTags,  DIP_X_LCE_O250             ,  m_LceExifInfo.i4O250);
    setIspIdx(rTags,  DIP_X_LCE_O500             ,  m_LceExifInfo.i4O500);
    setIspIdx(rTags,  DIP_X_LCE_O750             ,  m_LceExifInfo.i4O750);
    setIspIdx(rTags,  DIP_X_LCE_O950             ,  m_LceExifInfo.i4O950);
    setIspIdx(rTags,  DIP_X_LCE_O999             ,  m_LceExifInfo.i4O999);
    setIspIdx(rTags,  DIP_X_LCE_CenMaxSlope      ,  m_LceExifInfo.i4CenMaxSlope);
    setIspIdx(rTags,  DIP_X_LCE_CenMinSlope      ,  m_LceExifInfo.i4CenMinSlope);
    setIspIdx(rTags,  DIP_X_LCE_LCE_FD_Enable    ,  m_LceExifInfo.i4LCE_FD_Enable);
    setIspIdx(rTags,  DIP_X_LCE_FaceNum          ,  m_LceExifInfo.i4FaceNum);
    setIspIdx(rTags,  DIP_X_LCE_LCSXLow          ,  m_LceExifInfo.i4LCSXLow);
    setIspIdx(rTags,  DIP_X_LCE_LCSXHi           ,  m_LceExifInfo.i4LCSXHi);
    setIspIdx(rTags,  DIP_X_LCE_LCSYLow          ,  m_LceExifInfo.i4LCSYLow);
    setIspIdx(rTags,  DIP_X_LCE_LCSYHi           ,  m_LceExifInfo.i4LCSYHi);
    setIspIdx(rTags,  DIP_X_LCE_FaceLoBound      ,  m_LceExifInfo.i4FaceLoBound);
    setIspIdx(rTags,  DIP_X_LCE_FaceHiBound      ,  m_LceExifInfo.i4FaceHiBound);
    setIspIdx(rTags,  DIP_X_LCE_ctrlPoint_f0     ,  m_LceExifInfo.i4ctrlPoint_f0);
    setIspIdx(rTags,  DIP_X_LCE_ctrlPoint_f1     ,  m_LceExifInfo.i4ctrlPoint_f1);
    setIspIdx(rTags,  DIP_X_LCE_FDY              ,  m_LceExifInfo.i4FDY);
    setIspIdx(rTags,  DIP_X_LCE_MeterFDTarget    ,  m_LceExifInfo.i4MeterFDTarget);
    setIspIdx(rTags,  DIP_X_LCE_FDProb           ,  m_LceExifInfo.i4FDProb);
    setIspIdx(rTags,  DIP_X_LCE_AEGain           ,  m_LceExifInfo.i4AEGain);
    setIspIdx(rTags,  DIP_X_LCE_KeepBrightEnable ,  m_LceExifInfo.i4KeepBrightEnable);
    setIspIdx(rTags,  DIP_X_LCE_currBrightSlope  ,  m_LceExifInfo.i4currBrightSlope);
    setIspIdx(rTags,  DIP_X_LCE_OriP0            ,  m_LceExifInfo.i4OriP0);
    setIspIdx(rTags,  DIP_X_LCE_OriP1            ,  m_LceExifInfo.i4OriP1);
    setIspIdx(rTags,  DIP_X_LCE_OriP50           ,  m_LceExifInfo.i4OriP50);
    setIspIdx(rTags,  DIP_X_LCE_OriP250          ,  m_LceExifInfo.i4OriP250);
    setIspIdx(rTags,  DIP_X_LCE_OriP500          ,  m_LceExifInfo.i4OriP500);
    setIspIdx(rTags,  DIP_X_LCE_OriP750          ,  m_LceExifInfo.i4OriP750);
    setIspIdx(rTags,  DIP_X_LCE_OriP950          ,  m_LceExifInfo.i4OriP950);
    setIspIdx(rTags,  DIP_X_LCE_OriP999          ,  m_LceExifInfo.i4OriP999);
    setIspIdx(rTags,  DIP_X_LCE_OriO0            ,  m_LceExifInfo.i4OriO0);
    setIspIdx(rTags,  DIP_X_LCE_OriO1            ,  m_LceExifInfo.i4OriO1);
    setIspIdx(rTags,  DIP_X_LCE_OriO50           ,  m_LceExifInfo.i4OriO50);
    setIspIdx(rTags,  DIP_X_LCE_OriO250          ,  m_LceExifInfo.i4OriO250);
    setIspIdx(rTags,  DIP_X_LCE_OriO500          ,  m_LceExifInfo.i4OriO500);
    setIspIdx(rTags,  DIP_X_LCE_OriO750          ,  m_LceExifInfo.i4OriO750);
    setIspIdx(rTags,  DIP_X_LCE_OriO950          ,  m_LceExifInfo.i4OriO950);
    setIspIdx(rTags,  DIP_X_LCE_OriO999          ,  m_LceExifInfo.i4OriO999);
    setIspIdx(rTags,  DIP_X_LCE_FaceP0           ,  m_LceExifInfo.i4FaceP0);
    setIspIdx(rTags,  DIP_X_LCE_FaceP1           ,  m_LceExifInfo.i4FaceP1);
    setIspIdx(rTags,  DIP_X_LCE_FaceP50          ,  m_LceExifInfo.i4FaceP50);
    setIspIdx(rTags,  DIP_X_LCE_FaceP250         ,  m_LceExifInfo.i4FaceP250);
    setIspIdx(rTags,  DIP_X_LCE_FaceP500         ,  m_LceExifInfo.i4FaceP500);
    setIspIdx(rTags,  DIP_X_LCE_FaceP750         ,  m_LceExifInfo.i4FaceP750);
    setIspIdx(rTags,  DIP_X_LCE_FaceP950         ,  m_LceExifInfo.i4FaceP950);
    setIspIdx(rTags,  DIP_X_LCE_FaceP999         ,  m_LceExifInfo.i4FaceP999);
    setIspIdx(rTags,  DIP_X_LCE_FaceO0           ,  m_LceExifInfo.i4FaceO0);
    setIspIdx(rTags,  DIP_X_LCE_FaceO1           ,  m_LceExifInfo.i4FaceO1);
    setIspIdx(rTags,  DIP_X_LCE_FaceO50          ,  m_LceExifInfo.i4FaceO50);
    setIspIdx(rTags,  DIP_X_LCE_FaceO250         ,  m_LceExifInfo.i4FaceO250);
    setIspIdx(rTags,  DIP_X_LCE_FaceO500         ,  m_LceExifInfo.i4FaceO500);
    setIspIdx(rTags,  DIP_X_LCE_FaceO750         ,  m_LceExifInfo.i4FaceO750);
    setIspIdx(rTags,  DIP_X_LCE_FaceO950         ,  m_LceExifInfo.i4FaceO950);
    setIspIdx(rTags,  DIP_X_LCE_FaceO999         ,  m_LceExifInfo.i4FaceO999);
    setIspIdx(rTags,  DIP_X_LCE_SmoothEnable     ,  m_LceExifInfo.i4SmoothEnable);
    setIspIdx(rTags,  DIP_X_LCE_SmoothSpeed      ,  m_LceExifInfo.i4SmoothSpeed);
    setIspIdx(rTags,  DIP_X_LCE_SmoothWaitAE     ,  m_LceExifInfo.i4SmoothWaitAE);
    setIspIdx(rTags,  DIP_X_LCE_FlareEnable      ,  m_LceExifInfo.i4FlareEnable);
    setIspIdx(rTags,  DIP_X_LCE_FlareOffset      ,  m_LceExifInfo.i4FlareOffset);
    setIspIdx(rTags,  DIP_X_LCE_HalfLVIdx_L      ,  m_LceExifInfo.i4HalfLVIdx_L);
    setIspIdx(rTags,  DIP_X_LCE_HalfLVIdx_H      ,  m_LceExifInfo.i4HalfLVIdx_H);
    setIspIdx(rTags,  DIP_X_LCE_CurrDR           ,  m_LceExifInfo.i4CurrDR);
    setIspIdx(rTags,  DIP_X_LCE_DRIdx_L          ,  m_LceExifInfo.i4DRIdx_L);
    setIspIdx(rTags,  DIP_X_LCE_DRIdx_H          ,  m_LceExifInfo.i4DRIdx_H);
    setIspIdx(rTags,  DIP_X_LCE_FaceDCEFlag      ,  m_LceExifInfo.bFaceDCEFlag);
    setIspIdx(rTags,  DIP_X_LCE_DarkHEWt         ,  m_LceExifInfo.i4DarkHEWt);
    setIspIdx(rTags,  DIP_X_LCE_BrightHEWt       ,  m_LceExifInfo.i4BrightHEWt);
    setIspIdx(rTags,  DIP_X_LCE_SkyThr           ,  m_LceExifInfo.i4SkyThr);
    setIspIdx(rTags,  DIP_X_LCE_DceSkyLimitThr   ,  m_LceExifInfo.i4DceSkyLimitThr);
    setIspIdx(rTags,  DIP_X_LCE_SkyOn            ,  m_LceExifInfo.i4SkyOn);
    setIspIdx(rTags,  DIP_X_LCE_SkyOff           ,  m_LceExifInfo.i4SkyOff);
    setIspIdx(rTags,  DIP_X_LCE_DCESlopeMax      ,  m_LceExifInfo.i4DCESlopeMax);
    setIspIdx(rTags,  DIP_X_LCE_DCESmoothSpeed   ,  m_LceExifInfo.i4DCESmoothSpeed);
    setIspIdx(rTags,  DIP_X_LCE_IncorrectLCSO    ,  m_LceExifInfo.bIncorrectLCSO);
    setIspIdx(rTags,  DIP_X_LCE_DCEP0            ,  m_LceExifInfo.i4DCEP0);
    setIspIdx(rTags,  DIP_X_LCE_DCEP1            ,  m_LceExifInfo.i4DCEP1);
    setIspIdx(rTags,  DIP_X_LCE_DCEP50           ,  m_LceExifInfo.i4DCEP50);
    setIspIdx(rTags,  DIP_X_LCE_DCEP250          ,  m_LceExifInfo.i4DCEP250);
    setIspIdx(rTags,  DIP_X_LCE_DCEP500          ,  m_LceExifInfo.i4DCEP500);
    setIspIdx(rTags,  DIP_X_LCE_DCEP750          ,  m_LceExifInfo.i4DCEP750);
    setIspIdx(rTags,  DIP_X_LCE_DCEP950          ,  m_LceExifInfo.i4DCEP950);
    setIspIdx(rTags,  DIP_X_LCE_DCEP999          ,  m_LceExifInfo.i4DCEP999);
    setIspIdx(rTags,  DIP_X_LCE_DCEO0            ,  m_LceExifInfo.i4DCEO0);
    setIspIdx(rTags,  DIP_X_LCE_DCEO1            ,  m_LceExifInfo.i4DCEO1);
    setIspIdx(rTags,  DIP_X_LCE_DCEO50           ,  m_LceExifInfo.i4DCEO50);
    setIspIdx(rTags,  DIP_X_LCE_DCEO250          ,  m_LceExifInfo.i4DCEO250);
    setIspIdx(rTags,  DIP_X_LCE_DCEO500          ,  m_LceExifInfo.i4DCEO500);
    setIspIdx(rTags,  DIP_X_LCE_DCEO750          ,  m_LceExifInfo.i4DCEO750);
    setIspIdx(rTags,  DIP_X_LCE_DCEO950          ,  m_LceExifInfo.i4DCEO950);
    setIspIdx(rTags,  DIP_X_LCE_DCEO999          ,  m_LceExifInfo.i4DCEO999);
    setIspIdx(rTags,  DIP_X_LCE_IDX, (MUINT32)m_pLceMgr->getIDX());

    setIspIdx(rTags,  FACE_NUM       ,   m_fd_tuning_info.FaceNum);
    setIspIdx(rTags,  FACE_1_1       ,   m_fd_tuning_info.YUVsts[0][0]);
    setIspIdx(rTags,  FACE_1_2       ,   m_fd_tuning_info.YUVsts[0][1]);
    setIspIdx(rTags,  FACE_1_3       ,   m_fd_tuning_info.YUVsts[0][2]);
    setIspIdx(rTags,  FACE_1_4       ,   m_fd_tuning_info.YUVsts[0][3]);
    setIspIdx(rTags,  FACE_1_5       ,   m_fd_tuning_info.YUVsts[0][4]);
    setIspIdx(rTags,  FACE_1_6       ,   m_fd_tuning_info.fld_GenderLabel[0]);

    setIspIdx(rTags,  FACE_2_1       ,   m_fd_tuning_info.YUVsts[1][0]);
    setIspIdx(rTags,  FACE_2_2       ,   m_fd_tuning_info.YUVsts[1][1]);
    setIspIdx(rTags,  FACE_2_3       ,   m_fd_tuning_info.YUVsts[1][2]);
    setIspIdx(rTags,  FACE_2_4       ,   m_fd_tuning_info.YUVsts[1][3]);
    setIspIdx(rTags,  FACE_2_5       ,   m_fd_tuning_info.YUVsts[1][4]);
    setIspIdx(rTags,  FACE_2_6       ,   m_fd_tuning_info.fld_GenderLabel[1]);

    setIspIdx(rTags,  FACE_3_1       ,   m_fd_tuning_info.YUVsts[2][0]);
    setIspIdx(rTags,  FACE_3_2       ,   m_fd_tuning_info.YUVsts[2][1]);
    setIspIdx(rTags,  FACE_3_3       ,   m_fd_tuning_info.YUVsts[2][2]);
    setIspIdx(rTags,  FACE_3_4       ,   m_fd_tuning_info.YUVsts[2][3]);
    setIspIdx(rTags,  FACE_3_5       ,   m_fd_tuning_info.YUVsts[2][4]);
    setIspIdx(rTags,  FACE_3_6       ,   m_fd_tuning_info.fld_GenderLabel[2]);

    setIspIdx(rTags,  FACE_4_1       ,   m_fd_tuning_info.YUVsts[3][0]);
    setIspIdx(rTags,  FACE_4_2       ,   m_fd_tuning_info.YUVsts[3][1]);
    setIspIdx(rTags,  FACE_4_3       ,   m_fd_tuning_info.YUVsts[3][2]);
    setIspIdx(rTags,  FACE_4_4       ,   m_fd_tuning_info.YUVsts[3][3]);
    setIspIdx(rTags,  FACE_4_5       ,   m_fd_tuning_info.YUVsts[3][4]);
    setIspIdx(rTags,  FACE_4_6       ,   m_fd_tuning_info.fld_GenderLabel[3]);

    setIspIdx(rTags,  FACE_5_1       ,   m_fd_tuning_info.YUVsts[4][0]);
    setIspIdx(rTags,  FACE_5_2       ,   m_fd_tuning_info.YUVsts[4][1]);
    setIspIdx(rTags,  FACE_5_3       ,   m_fd_tuning_info.YUVsts[4][2]);
    setIspIdx(rTags,  FACE_5_4       ,   m_fd_tuning_info.YUVsts[4][3]);
    setIspIdx(rTags,  FACE_5_5       ,   m_fd_tuning_info.YUVsts[4][4]);
    setIspIdx(rTags,  FACE_5_6       ,   m_fd_tuning_info.fld_GenderLabel[4]);

    ////////////////////////////////////////////////////////////////////////////
    //  (2.2) ISPComm
    ////////////////////////////////////////////////////////////////////////////
    for (MUINT32 i = 0; i < sizeof(ISP_NVRAM_COMMON_STRUCT)/sizeof(MUINT32); i++)
    {
        MUINT32 const u4TagID = COMM_Begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = m_rIspComm.CommReg[i];
    }
    //  (2.3) RAWIspCamInfo
    CAM_LOGD_IF(m_bDebugEnable,"RAWIspCamInfo:");
    setIspIdx(rTags, isHDR_P2,      rCamInfo.fgHDR);
    setIspIdx(rTags, SceneIdx_P2, rCamInfo.eIdx_Scene);
    setIspIdx(rTags, UPPER_ISO_IDX_P2, rCamInfo.eIdx_ISO_U);
    setIspIdx(rTags, LOWER_ISO_IDX_P2, rCamInfo.eIdx_ISO_L);
    setIspIdx(rTags, UPPER_ZOOM_IDX_P2,  rCamInfo.eIdx_Zoom_U);
    setIspIdx(rTags, LOWER_ZOOM_IDX_P2,  rCamInfo.eIdx_Zoom_L);
    setIspIdx(rTags, UPPER_CT_IDX_P2,    rCamInfo.eIdx_CT_U);
    setIspIdx(rTags, LOWER_CT_IDX_P2,    rCamInfo.eIdx_CT_L);
    setIspIdx(rTags, ZoomRatio_x100_P2, rCamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, ShadingIdx_P2, rCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, Custom_Interp_P2, m_pIspTuningCustom->is_to_invoke_isp_interpolation(rCamInfo));


    //
    //  (2.4) EffectMode
    CAM_LOGD_IF(m_bDebugEnable,"EffectMode:");
    setIspIdx(rTags, EffectMode, rCamInfo.eIdx_Effect);
    //
    //  (2.5) UserSelectLevel
    CAM_LOGD_IF(m_bDebugEnable,"UserSelectLevel:");
    setIspIdx(rTags, EdgeIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Edge);
    setIspIdx(rTags, HueIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Hue);
    setIspIdx(rTags, SatIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Sat);
    setIspIdx(rTags, BrightIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Bright);
    setIspIdx(rTags, ContrastIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Contrast);
    //AE Info
    setIspIdx(rTags, AE_INFO_EXP_TIME_P2, rCamInfo.rAEInfo.u4Eposuretime);
    setIspIdx(rTags, AE_INFO_AFE_GAIN_P2, rCamInfo.rAEInfo.u4AfeGain);
    setIspIdx(rTags, AE_INFO_ISP_GAIN_P2, rCamInfo.rAEInfo.u4IspGain);
    setIspIdx(rTags, AE_INFO_REAL_ISO_P2, rCamInfo.rAEInfo.u4RealISOValue);
    setIspIdx(rTags, AE_INFO_LV_x10_P2, rCamInfo.rAEInfo.i4RealLightValue_x10);

    MUINT32 dbg_OBC2_AEGain;
    ISP_MGR_OBC2_T::getInstance(m_eSensorDev).getIspAEGain(&dbg_OBC2_AEGain);
    setIspIdx(rTags, OBC2_AE_GAIN, dbg_OBC2_AEGain);

    MINT32 dbg_PGN_FlareGain, dbg_PGN_FlareOfst;
    ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspFlare(&dbg_PGN_FlareGain, &dbg_PGN_FlareOfst);
    setIspIdx(rTags, PGN_FLARE_GAIN, dbg_PGN_FlareGain);
    setIspIdx(rTags, PGN_FLARE_OFST, dbg_PGN_FlareOfst);

    //AWB Info
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_R_P2, rCamInfo.rAWBInfo.rCurrentAWBGain.i4R);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_G_P2, rCamInfo.rAWBInfo.rCurrentAWBGain.i4G);
    setIspIdx(rTags, AWB_INFO_CURRENT_GAIN_B_P2, rCamInfo.rAWBInfo.rCurrentAWBGain.i4B);
    setIspIdx(rTags, AWB_INFO_CT_P2, rCamInfo.rAWBInfo.i4CCT);

    setIspIdx(rTags, HighQCap, rCamInfo.fgNeedKeepP1);


    MUINT32 p2Size = sizeof(dip_x_reg_t);
    if (p2Size/4 > P2_BUFFER_SIZE)
    {
        CAM_LOGE("P2 buffer:(%d) byte exceed EXIF allocation size:(%d) byte", p2Size, P2_BUFFER_SIZE*4);
    }
    else
    {
        memcpy(rDebugInfo.P2RegInfo.regDataP2, pRegBuf, p2Size);
        rDebugInfo.P2RegInfo.u4TableSize = P2_BUFFER_SIZE+1;
        rDebugInfo.P2RegInfo.u4HwVersion = 5;
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
            rDebugInfo.MFBRegInfo.u4HwVersion = 5;
        }
    }

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setDebugInfo4TG(MUINT32 const u4Rto, MINT32 const OBCGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    MUINT32 u4OBCGainOffset;
    MUINT32 u4RtoOffset;
    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN0))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN1))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN2))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN3))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4RtoOffset = ((MUINT32)offsetof(cam_reg_t, CAM_LSC_RATIO_0))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

    u4RtoOffset = ((MUINT32)offsetof(cam_reg_t, CAM_LSC_RATIO_1))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setDebugInfo4CCU(MUINT32 const u4Rto, ISP_NVRAM_OBC_T const &OBCResult, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    rDbgIspInfo.debugInfo.tags[CCUEnable].u4Val = 1;

    MUINT32 u4OBCGainOffset;
    MUINT32 u4OBCOFFSTOffset;
    MUINT32 u4RtoOffset;

    // OBC Gain
    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN0))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain0.val;

    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN1))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain1.val;

    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN2))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain2.val;

    u4OBCGainOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_GAIN3))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain3.val;

    // OBC OFFST
    u4OBCOFFSTOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_OFFST0))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) = OBCResult.offst0.val;

    u4OBCOFFSTOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_OFFST1))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) =  OBCResult.offst1.val;

    u4OBCOFFSTOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_OFFST2))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) = OBCResult.offst2.val;

    u4OBCOFFSTOffset = ((MUINT32)offsetof(cam_reg_t, CAM_OBC_OFFST3))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) = OBCResult.offst3.val;

    // Ratio
    u4RtoOffset = ((MUINT32)offsetof(cam_reg_t, CAM_LSC_RATIO_0))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

    u4RtoOffset = ((MUINT32)offsetof(cam_reg_t, CAM_LSC_RATIO_1))/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

    return  MERR_OK;
}

