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
#include <isp_tuning_idx.h>
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
    MUINT32 const u4Idx
)
{
    rTags[eTagID].u4ID  = getIspTag(eTagID);
    rTags[eTagID].u4Val = u4Idx;
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

    // RAWIspCamInfo
    setIspIdx(rTags, IspProfile_P1,      m_rIspCamInfo.eIspProfile);
    setIspIdx(rTags, isHDR_P1,           m_rIspCamInfo.fgHDR);
    setIspIdx(rTags, SensorMode_P1,      m_rIspCamInfo.eSensorMode);
    setIspIdx(rTags, SceneIdx_P1,        m_rIspCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOIdx_P1,          m_rIspCamInfo.eIdx_ISO);
    setIspIdx(rTags, UPPER_ISO_IDX_P1,   m_rIspCamInfo.eIdx_ISO_U);
    setIspIdx(rTags, LOWER_ISO_IDX_P1,   m_rIspCamInfo.eIdx_ISO_L);
    setIspIdx(rTags, UPPER_ZOOM_IDX_P1,  m_rIspCamInfo.eIdx_Zoom_U);
    setIspIdx(rTags, LOWER_ZOOM_IDX_P1,  m_rIspCamInfo.eIdx_Zoom_L);
    setIspIdx(rTags, CTIdx_P1,           m_rIspCamInfo.eIdx_CT);
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
    setIspIdx(rTags, AE_INFO_LV_x10_P1, m_rIspCamInfo.rAEInfo.i4LightValue_x10);

    MUINT32 dbg_OBC_AEGain;
    ISP_MGR_OBC_T::getInstance(m_eSensorDev).getIspAEGain(&dbg_OBC_AEGain);
    setIspIdx(rTags, OBC_AE_GAIN, dbg_OBC_AEGain);

    MINT32 dbg_RPG_FlareGain, dbg_RPG_FlareOfst;
    ISP_MGR_RPG_T::getInstance(m_eSensorDev).getIspFlare(&dbg_RPG_FlareGain, &dbg_RPG_FlareOfst);
    setIspIdx(rTags, RPG_FLARE_GAIN, dbg_RPG_FlareGain);
    setIspIdx(rTags, RPG_FLARE_OFST, dbg_RPG_FlareOfst);

    //Flash Info
    setIspIdx(rTags, FLASH_INFO_Enable_P1, m_rIspCamInfo.rFlashInfo.isFlash);

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
            memcpy(rDebugInfo.P1RegInfo.regData, pP1TuningBuffer, u4RegCnt*4);
            memcpy(rDebugInfo.P1RegInfo.regData+u4RRZOfst, &m_RRZInfo, sizeof(ISP_NVRAM_RRZ_T)); //overwrite rrz register by member
            rDebugInfo.P1RegInfo.u4TableSize = P1_BUFFER_SIZE+1; //add u4HwVersion
            rDebugInfo.P1RegInfo.u4HwVersion = 4;
        }
    }

    // CCM P1
    setIspIdx(rTags, DYNAMIC_CCM_IDX_P1, m_ISP_DYM.sCcm_tbl_P1_Idx);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfo(const NSIspTuning::ISP_INFO_T& rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, const void* pRegBuf) const
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
    setIspIdx(rTags, CCM_Method, m_rIspParam.ISPColorTbl.ISPMulitCCM.CCM_Method);
    setIspIdx(rTags, SMOOTH_CCM_PQType, m_rIspParam.ISPColorTbl.ISPMulitCCM.PQ_Type);
    setIspIdx(rTags, SMOOTH_CCM_LV_NUM, m_rIspParam.ISPColorTbl.ISPMulitCCM.CCM_LV_valid_NUM);
    setIspIdx(rTags, SMOOTH_CCM_FLASH_NUM, m_rIspParam.ISPColorTbl.ISPMulitCCM.CCM_FLASH_valid_NUM);
    setIspIdx(rTags, SMOOTH_CCM_CT_NUM, m_rIspParam.ISPColorTbl.ISPMulitCCM.CCM_CT_valid_NUM);
    setIspIdx(rTags, SMOOTH_CCM_Ver, m_ISP_ALG_VER.sCCM);

    //PCA & COLOR
    setIspIdx(rTags, COLOR_Method, m_rIspParam.ISPColorTbl.ISPColor.COLOR_Method);
    setIspIdx(rTags, SMOOTH_PCA_Ver, m_ISP_ALG_VER.sPCA);
    setIspIdx(rTags, SMOOTH_COLOR_Ver, m_ISP_ALG_VER.sCOLOR);

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

    //dynamic Gamma
    // adaptive Gamma
    CAM_LOGD_IF(m_bDebugEnable,"adaptive Gamma:");
    setIspIdx(rTags, DIP_X_GMA_GMAMode, m_GmaExifInfo.i4GMAMode);
    setIspIdx(rTags, DIP_X_GMA_GMACamMode, m_GmaExifInfo.i4GMACamMode);
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

    //dynamic LCE
    // adaptive LCE
    CAM_LOGD_IF(m_bDebugEnable,"adaptive LCE:");
    setIspIdx(rTags,  DIP_X_LCE_LceProfile       ,  m_LceExifInfo.i4LceProfile);
    setIspIdx(rTags,  DIP_X_LCE_LCECamMode       ,  m_LceExifInfo.i4LCECamMode);
    setIspIdx(rTags,  DIP_X_LCE_AutoHDREnable    ,  m_LceExifInfo.i4AutoHDREnable);
    setIspIdx(rTags,  DIP_X_LCE_ChipVersion      ,  m_LceExifInfo.i4ChipVersion);
    setIspIdx(rTags,  DIP_X_LCE_MainVersion      ,  m_LceExifInfo.i4MainVersion);
    setIspIdx(rTags,  DIP_X_LCE_SubVersion       ,  m_LceExifInfo.i4SubVersion);
    setIspIdx(rTags,  DIP_X_LCE_SystemVersion    ,  m_LceExifInfo.i4SystemVersion);
    setIspIdx(rTags,  DIP_X_LCE_LV               ,  m_LceExifInfo.i4LV);
    setIspIdx(rTags,  DIP_X_LCE_ContrastY10      ,  m_LceExifInfo.i4ContrastY10);
    setIspIdx(rTags,  DIP_X_LCE_EVRatio          ,  m_LceExifInfo.i4EVRatio);
    setIspIdx(rTags,  DIP_X_LCE_EVContrastY10    ,  m_LceExifInfo.i4EVContrastY10);
    setIspIdx(rTags,  DIP_X_LCE_SegDiv           ,  m_LceExifInfo.i4SegDiv);
    setIspIdx(rTags,  DIP_X_LCE_ContrastIdx_L    ,  m_LceExifInfo.i4ContrastIdx_L);
    setIspIdx(rTags,  DIP_X_LCE_ContrastIdx_H    ,  m_LceExifInfo.i4ContrastIdx_H);
    setIspIdx(rTags,  DIP_X_LCE_LVIdx_L          ,  m_LceExifInfo.i4LVIdx_L);
    setIspIdx(rTags,  DIP_X_LCE_LVIdx_H          ,  m_LceExifInfo.i4LVIdx_H);
    setIspIdx(rTags,  DIP_X_LCE_DetailRatio1     ,  m_LceExifInfo.i4DetailRatio1);
    setIspIdx(rTags,  DIP_X_LCE_DetailRatio50    ,  m_LceExifInfo.i4DetailRatio50);
    setIspIdx(rTags,  DIP_X_LCE_DetailRatio500   ,  m_LceExifInfo.i4DetailRatio500);
    setIspIdx(rTags,  DIP_X_LCE_DetailRatio950   ,  m_LceExifInfo.i4DetailRatio950);
    setIspIdx(rTags,  DIP_X_LCE_DetailRatio999   ,  m_LceExifInfo.i4DetailRatio999);
    setIspIdx(rTags,  DIP_X_LCE_IntpDiffRangex128,  m_LceExifInfo.i4IntpDiffRange128);
    setIspIdx(rTags,  DIP_X_LCE_DiffRangeIdx_L   ,  m_LceExifInfo.i4DiffRangeIdx_L);
    setIspIdx(rTags,  DIP_X_LCE_DiffRangeIdx_H   ,  m_LceExifInfo.i4DiffRangeIdx_H);
    setIspIdx(rTags,  DIP_X_LCE_P1               ,  m_LceExifInfo.i4P1);
    setIspIdx(rTags,  DIP_X_LCE_P50              ,  m_LceExifInfo.i4P50);
    setIspIdx(rTags,  DIP_X_LCE_P500             ,  m_LceExifInfo.i4P500);
    setIspIdx(rTags,  DIP_X_LCE_P950             ,  m_LceExifInfo.i4P950);
    setIspIdx(rTags,  DIP_X_LCE_P999             ,  m_LceExifInfo.i4P999);
    setIspIdx(rTags,  DIP_X_LCE_O1               ,  m_LceExifInfo.i4O1);
    setIspIdx(rTags,  DIP_X_LCE_O50              ,  m_LceExifInfo.i4O50);
    setIspIdx(rTags,  DIP_X_LCE_O500             ,  m_LceExifInfo.i4O500);
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
    setIspIdx(rTags,  DIP_X_LCE_Ori_P1           ,  m_LceExifInfo.i4OriP1);
    setIspIdx(rTags,  DIP_X_LCE_Ori_P50          ,  m_LceExifInfo.i4OriP50);
    setIspIdx(rTags,  DIP_X_LCE_Ori_P500         ,  m_LceExifInfo.i4OriP500);
    setIspIdx(rTags,  DIP_X_LCE_Ori_P950         ,  m_LceExifInfo.i4OriP950);
    setIspIdx(rTags,  DIP_X_LCE_Ori_P999         ,  m_LceExifInfo.i4OriP999);
    setIspIdx(rTags,  DIP_X_LCE_Ori_O1           ,  m_LceExifInfo.i4OriO1);
    setIspIdx(rTags,  DIP_X_LCE_Ori_O50          ,  m_LceExifInfo.i4OriO50);
    setIspIdx(rTags,  DIP_X_LCE_Ori_O500         ,  m_LceExifInfo.i4OriO500);
    setIspIdx(rTags,  DIP_X_LCE_Ori_O950         ,  m_LceExifInfo.i4OriO950);
    setIspIdx(rTags,  DIP_X_LCE_Ori_O999         ,  m_LceExifInfo.i4OriO999);
    setIspIdx(rTags,  DIP_X_LCE_Face_P1          ,  m_LceExifInfo.i4FaceP1);
    setIspIdx(rTags,  DIP_X_LCE_Face_P50         ,  m_LceExifInfo.i4FaceP50);
    setIspIdx(rTags,  DIP_X_LCE_Face_P500        ,  m_LceExifInfo.i4FaceP500);
    setIspIdx(rTags,  DIP_X_LCE_Face_P950        ,  m_LceExifInfo.i4FaceP950);
    setIspIdx(rTags,  DIP_X_LCE_Face_P999        ,  m_LceExifInfo.i4FaceP999);
    setIspIdx(rTags,  DIP_X_LCE_Face_O1          ,  m_LceExifInfo.i4FaceO1);
    setIspIdx(rTags,  DIP_X_LCE_Face_O50         ,  m_LceExifInfo.i4FaceO50);
    setIspIdx(rTags,  DIP_X_LCE_Face_O500        ,  m_LceExifInfo.i4FaceO500);
    setIspIdx(rTags,  DIP_X_LCE_Face_O950        ,  m_LceExifInfo.i4FaceO950);
    setIspIdx(rTags,  DIP_X_LCE_Face_O999        ,  m_LceExifInfo.i4FaceO999);
    setIspIdx(rTags,  DIP_X_LCE_SmoothEnable     ,  m_LceExifInfo.i4SmoothEnable);
    setIspIdx(rTags,  DIP_X_LCE_SmoothSpeed      ,  m_LceExifInfo.i4SmoothSpeed);
    setIspIdx(rTags,  DIP_X_LCE_SmoothWaitAE     ,  m_LceExifInfo.i4SmoothWaitAE);
    setIspIdx(rTags,  DIP_X_LCE_FlareEnable      ,  m_LceExifInfo.i4FlareEnable);
    setIspIdx(rTags,  DIP_X_LCE_FlareOffset      ,  m_LceExifInfo.i4FlareOffset);

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
    setIspIdx(rTags, IspProfile_P2, rCamInfo.eIspProfile);
    setIspIdx(rTags, isHDR_P2,      rCamInfo.fgHDR);
    setIspIdx(rTags, SensorMode_P2, rCamInfo.eSensorMode);
    setIspIdx(rTags, SceneIdx_P2, rCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOIdx_P2, rCamInfo.eIdx_ISO);
    setIspIdx(rTags, UPPER_ISO_IDX_P2, rCamInfo.eIdx_ISO_U);
    setIspIdx(rTags, LOWER_ISO_IDX_P2, rCamInfo.eIdx_ISO_L);
    setIspIdx(rTags, UPPER_ZOOM_IDX_P2,  rCamInfo.eIdx_Zoom_U);
    setIspIdx(rTags, LOWER_ZOOM_IDX_P2,  rCamInfo.eIdx_Zoom_L);
    setIspIdx(rTags, CTIdx_P2,           rCamInfo.eIdx_CT);
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
    setIspIdx(rTags, AE_INFO_LV_x10_P2, rCamInfo.rAEInfo.i4LightValue_x10);

    MUINT32 dbg_OBC2_AEGain;
    ISP_MGR_OBC2_T::getInstance(m_eSensorDev).getIspAEGain(&dbg_OBC2_AEGain);
    setIspIdx(rTags, OBC2_AE_GAIN, dbg_OBC2_AEGain);

    MINT32 dbg_PGN_FlareGain, dbg_PGN_FlareOfst;
    ISP_MGR_PGN_T::getInstance(m_eSensorDev).getIspFlare(&dbg_PGN_FlareGain, &dbg_PGN_FlareOfst);
    setIspIdx(rTags, PGN_FLARE_GAIN, dbg_PGN_FlareGain);
    setIspIdx(rTags, PGN_FLARE_OFST, dbg_PGN_FlareOfst);

    setIspIdx(rTags, ISO_THRESHOLD_ENABLE, m_bUseIsoThreshEnable);
    setIspIdx(rTags, ISO_THRESHOLD, m_isoThresh);
    setIspIdx(rTags, ISO_THRESHOLD_PREVIOUS_ISO, m_i4PreRealISO);

    //Flash Info
    setIspIdx(rTags, FLASH_INFO_Enable_P2, rCamInfo.rFlashInfo.isFlash);

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
        rDebugInfo.P2RegInfo.u4HwVersion = 4;
    }
    IspTuningBufCtrl::getInstance(m_eSensorDev)->updateHint((void*)(&rIspInfo.hint), rIspInfo.rCamInfo.u4Id);
    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfo_MultiPassNR(const NSIspTuning::ISP_INFO_T& rIspInfo, DEBUG_RESERVEA_INFO_T& rDebugInfo, const void* pRegBuf) const
{
    Mutex::Autolock lock(m_Lock);
    const MBOOL fgReadFromHW = MFALSE;

    const NSIspTuning::RAWIspCamInfo& rCamInfo = rIspInfo.rCamInfo;
    INDEX_T const*const pDefaultIndex = m_pIspTuningCustom->getDefaultIndex(
            rCamInfo.eIspProfile, rCamInfo.eSensorMode, rCamInfo.eIdx_Scene, rCamInfo.eIdx_ISO, rCamInfo.eIdx_Zoom);
    if  ( ! pDefaultIndex )
    {
        CAM_LOGE("[ERROR][getDebugInfo]pDefaultIndex==NULL");
        return MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND;
    }
    IndexMgr idxmgr = *pDefaultIndex;


    ssize_t idx = 0;
    #define addPair(debug_info, index, id, value)           \
        do{                                             \
            debug_info.Tag[index].u4FieldID = id;       \
            debug_info.Tag[index].u4FieldValue = value; \
            index++;                                    \
        } while(0)

    // ISP debug tag version
    addPair(rDebugInfo, idx, RESERVEA_TAG_VERSION, RESERVEA_DEBUG_TAG_VERSION);

    // ISP debug ISP profile
    addPair(rDebugInfo, idx, M_IspProfile, rCamInfo.eIspProfile);

    addPair(rDebugInfo, idx, M_IDX_ANR, idxmgr.getIdx_ANR());
    addPair(rDebugInfo, idx, M_ANR_UPPER_IDX, m_ISP_INT.sAnr.u2UpperIso_LowerZoom);
    addPair(rDebugInfo, idx, M_ANR_LOWER_IDX, m_ISP_INT.sAnr.u2LowerIso_LowerZoom);

    addPair(rDebugInfo, idx, M_IDX_ANR2, idxmgr.getIdx_ANR2());
    addPair(rDebugInfo, idx, M_ANR2_UPPER_IDX, m_ISP_INT.sAnr2.u2UpperIso_LowerZoom);
    addPair(rDebugInfo, idx, M_ANR2_LOWER_IDX, m_ISP_INT.sAnr2.u2LowerIso_LowerZoom);

    addPair(rDebugInfo, idx, M_IDX_CCR, idxmgr.getIdx_CCR());
    addPair(rDebugInfo, idx, M_CCR_UPPER_IDX, m_ISP_INT.sCcr.u2UpperIso_LowerZoom);
    addPair(rDebugInfo, idx, M_CCR_LOWER_IDX, m_ISP_INT.sCcr.u2LowerIso_LowerZoom);

    // ISP Top control
    ISP_NVRAM_CTL_EN_P2_T rCTL;
    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).get(rCTL, static_cast<const dip_x_reg_t*>(pRegBuf));
    addPair(rDebugInfo, idx, M_DIP_X_CTL_YUV_EN, rCTL.en_yuv.val);

    // ISP ANR registers
    ISP_NVRAM_ANR_T rANR;
    ISP_MGR_NBC_T::getInstance(m_eSensorDev).get(rANR, static_cast<const dip_x_reg_t*>(pRegBuf));
    int anr_begin = M_DIP_X_ANR_CON1;
    int anrSize = ISP_NVRAM_ANR_T::COUNT;
    for (int i=0; i<anrSize; i++)
        addPair(rDebugInfo, idx, anr_begin + i, rANR.set[i]);

    // ISP ANR2 registers
    ISP_NVRAM_ANR2_T rANR2;
    ISP_MGR_NBC2_T::getInstance(m_eSensorDev).get(rANR2, static_cast<const dip_x_reg_t*>(pRegBuf));
    int anr2_begin = M_DIP_X_ANR2_CON1;
    int anr2Size = ISP_NVRAM_ANR2_T::COUNT;
    for (int i=0; i<anr2Size; i++)
        addPair(rDebugInfo, idx, anr2_begin + i, rANR2.set[i]);

    // ISP CCR registers
    ISP_NVRAM_CCR_T rCCR;
    ISP_MGR_NBC2_T::getInstance(m_eSensorDev).get(rCCR, static_cast<const dip_x_reg_t*>(pRegBuf));
    int ccr_begin = M_DIP_X_CCR_CON;
    int ccrSize = ISP_NVRAM_CCR_T::COUNT;
    for (int i=0; i<ccrSize; i++)
        addPair(rDebugInfo, idx, ccr_begin + i, rCCR.set[i]);


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

