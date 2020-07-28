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

using namespace android;
using namespace NSFeature;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSIspExifDebug;


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
    MBOOL fgReadFromHW,
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
getDebugInfoP1(NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, MBOOL const fgReadFromHW) const
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

    // ISP Profile Pass1
    setIspIdx(rTags, IspProfile_P1, m_rIspCamInfo.eIspProfile);

    ////////////////////////////////////////////////////////////////////////////
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////

    // CAM_CTL_EN_P1
    CAM_LOGD_IF(m_bDebugEnable,"CAM_CTL_EN_P1:");
    setIspTags<ISP_NVRAM_CTL_EN_P1_T, CAM_CTL_EN_P1_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // SL2F
    CAM_LOGD_IF(m_bDebugEnable,"SL2F:");
    setIspIdx(rTags, IDX_SL2F, m_IspNvramMgr.getIdx_SL2F());
    setIspTags<ISP_NVRAM_SL2F_GET_T, CAM_SL2F_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // DBS
    CAM_LOGD_IF(m_bDebugEnable,"DBS:");
    setIspIdx(rTags, IDX_DBS, m_IspNvramMgr.getIdx_DBS());
    setIspIdx(rTags, DBS_UPPER_IDX, m_ISP_INT.sDbs.u2UpperIdx);
    setIspIdx(rTags, DBS_LOWER_IDX, m_ISP_INT.sDbs.u2LowerIdx);
    setIspTags<ISP_NVRAM_DBS_T, CAM_DBS_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // OBC
    CAM_LOGD_IF(m_bDebugEnable,"OBC:");
    setIspIdx(rTags, IDX_OBC, m_IspNvramMgr.getIdx_OBC());
    setIspIdx(rTags, OBC_UPPER_IDX, m_ISP_INT.sObc.u2UpperIdx);
    setIspIdx(rTags, OBC_LOWER_IDX, m_ISP_INT.sObc.u2LowerIdx);
    setIspTags<ISP_NVRAM_OBC_T, CAM_OBC_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // BPC
    CAM_LOGD_IF(m_bDebugEnable,"BPC:");
    setIspIdx(rTags, IDX_BPC, m_IspNvramMgr.getIdx_BPC());
    setIspIdx(rTags, BPC_UPPER_IDX, m_ISP_INT.sBpc.u2UpperIdx);
    setIspIdx(rTags, BPC_LOWER_IDX, m_ISP_INT.sBpc.u2LowerIdx);
    setIspTags<ISP_NVRAM_BNR_BPC_T, CAM_BPC_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // NR1
    CAM_LOGD_IF(m_bDebugEnable,"NR1:");
    setIspIdx(rTags, IDX_NR1, m_IspNvramMgr.getIdx_NR1());
    setIspIdx(rTags, NR1_UPPER_IDX, m_ISP_INT.sNr1.u2UpperIdx);
    setIspIdx(rTags, NR1_LOWER_IDX, m_ISP_INT.sNr1.u2LowerIdx);
    setIspTags<ISP_NVRAM_BNR_NR1_T, CAM_NR1_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // PDC
    CAM_LOGD_IF(m_bDebugEnable,"PDC:");
    setIspIdx(rTags, IDX_PDC, m_IspNvramMgr.getIdx_PDC());
    setIspIdx(rTags, PDC_UPPER_IDX, m_ISP_INT.sPdc.u2UpperIdx);
    setIspIdx(rTags, PDC_LOWER_IDX, m_ISP_INT.sPdc.u2LowerIdx);
    setIspTags<ISP_NVRAM_BNR_PDC_T, CAM_PDC_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // RMM
    CAM_LOGD_IF(m_bDebugEnable,"RMM:");
    setIspIdx(rTags, IDX_RMM, m_IspNvramMgr.getIdx_RMM());
    setIspIdx(rTags, RMM_UPPER_IDX, m_ISP_INT.sRmm.u2UpperIdx);
    setIspIdx(rTags, RMM_LOWER_IDX, m_ISP_INT.sRmm.u2LowerIdx);
    setIspTags<ISP_NVRAM_RMM_GET_T, CAM_RMM_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // RMG
    CAM_LOGD_IF(m_bDebugEnable,"RMG:");
    setIspTags<ISP_NVRAM_RMG_T, CAM_RMG_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // LSC
    CAM_LOGD_IF(m_bDebugEnable,"LSC:");
    setIspTags<ISP_NVRAM_LSC_T, CAM_LSC_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // RPG
    CAM_LOGD_IF(m_bDebugEnable,"RPG:");
    setIspTags<ISP_NVRAM_RPG_T, CAM_RPG_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    // LCS
    MY_LOG_IF(m_bDebugEnable,"LCS:");
    setIspTags<ISP_NVRAM_LCS_T, CAM_LCS_Begin>(m_eSensorDev, fgReadFromHW, rTags);

    return  MERR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
Paramctrl::
getDebugInfo(const NSIspTuning::ISP_INFO_T& _rIspInfo, NSIspExifDebug::IspExifDebugInfo_T& rDebugInfo, const void* pRegBuf) const
{
    Mutex::Autolock lock(m_Lock);
    const MBOOL fgReadFromHW = MFALSE;
    ////////////////////////////////////////////////////////////////////////////
    //  (1) Header.
    ////////////////////////////////////////////////////////////////////////////
    rDebugInfo.hdr  = g_rIspExifDebugInfoHdr;

    //zHDR ISPProfile remapping
    ISP_INFO_T rIspInfo_temp = _rIspInfo;
/*
        switch(rIspInfo_temp.rCamInfo.eIspProfile){
            case EIspProfile_zHDR_Preview:
                if (rIspInfo_temp.rCamInfo.eSensorMode == ESensorMode_Capture)
                {
                    rIspInfo_temp.rCamInfo.eIspProfile = EIspProfile_zHDR_Capture;
                }
                break;
            case EIspProfile_zHDR_Video:
                if (rIspInfo_temp.rCamInfo.eSensorMode == ESensorMode_Preview || m_bFrontBinEn)
                    // or frontal binning?
                {
                    rIspInfo_temp.rCamInfo.eIspProfile = EIspProfile_zHDR_Preview;
                }
                break;
            case EIspProfile_zHDR_Capture:
                if (rIspInfo_temp.rCamInfo.eSensorMode == ESensorMode_Preview || m_bFrontBinEn)  //use it?
                    // or frontal binning?
                {
                    rIspInfo_temp.rCamInfo.eIspProfile = EIspProfile_zHDR_Preview;
                }
                break;
            default:
                break;
        }

*/

        const ISP_INFO_T &rIspInfo = rIspInfo_temp;

    ////////////////////////////////////////////////////////////////////////////
    //  (2) Body.
    ////////////////////////////////////////////////////////////////////////////
    IspDebugTag (&rTags)[TagID_Total_Num] = rDebugInfo.debugInfo.tags;
    const NSIspTuning::RAWIspCamInfo& rCamInfo = rIspInfo.rCamInfo;

    // ISP debug tag version
    setIspIdx(rTags, IspTagVersion, IspDebugTagVersion);
    setIspIdx(rTags, UniqueKey, rIspInfo.i4UniqueKey);
    setIspIdx(rTags, AE_INFO_EXP_TIME, rCamInfo.rAEInfo.u4Eposuretime);
    setIspIdx(rTags, AE_INFO_AFE_GAIN, rCamInfo.rAEInfo.u4AfeGain);
    setIspIdx(rTags, AE_INFO_ISP_GAIN, rCamInfo.rAEInfo.u4IspGain);
    setIspIdx(rTags, AE_INFO_REAL_ISO, rCamInfo.rAEInfo.u4RealISOValue);
    ////////////////////////////////////////////////////////////////////////////
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////
    INDEX_T const*const pDefaultIndex = m_pIspTuningCustom->getDefaultIndex(
            rCamInfo.eIspProfile, rCamInfo.eSensorMode, rCamInfo.eIdx_Scene, rCamInfo.eIdx_ISO);
    if  ( ! pDefaultIndex )
    {
        CAM_LOGE("[ERROR][getDebugInfo]pDefaultIndex==NULL");
        return MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND;
    }
    IndexMgr idxmgr = *pDefaultIndex;

    // CAM_CTL_EN_P2
    CAM_LOGD_IF(m_bDebugEnable,"CAM_CTL_EN_P2:");
    setIspTags2<ISP_NVRAM_CTL_EN_P2_T, ISP_MGR_CTL_EN_P2_T, CAM_CTL_EN_P2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // SL2G
    CAM_LOGD_IF(m_bDebugEnable,"SL2G:");
    setIspIdx(rTags, IDX_SL2F, idxmgr.getIdx_SL2F());
    setIspTags2<ISP_NVRAM_SL2G_GET_T, ISP_MGR_SL2G_T, DIP_X_SL2G_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // DBS2
    CAM_LOGD_IF(m_bDebugEnable,"DBS2:");
    setIspIdx(rTags, IDX_DBS2, idxmgr.getIdx_DBS());
    setIspIdx(rTags, DBS2_UPPER_IDX, m_ISP_INT.sDbs2.u2UpperIdx);
    setIspIdx(rTags, DBS2_LOWER_IDX, m_ISP_INT.sDbs2.u2LowerIdx);
    setIspTags2<ISP_NVRAM_DBS_T, ISP_MGR_DBS2_T, DIP_X_DBS2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // OBC2
    CAM_LOGD_IF(m_bDebugEnable,"OBC2:");
    setIspIdx(rTags, IDX_OBC2, idxmgr.getIdx_OBC());
    setIspIdx(rTags, OBC2_UPPER_IDX, m_ISP_INT.sObc2.u2UpperIdx);
    setIspIdx(rTags, OBC2_LOWER_IDX, m_ISP_INT.sObc2.u2LowerIdx);
    setIspTags2<ISP_NVRAM_OBC_T, ISP_MGR_OBC2_T, DIP_X_OBC2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // BPC2
    CAM_LOGD_IF(m_bDebugEnable,"BPC2:");
    setIspIdx(rTags, IDX_BPC2, idxmgr.getIdx_BPC());
    setIspIdx(rTags, BPC2_UPPER_IDX, m_ISP_INT.sBpc2.u2UpperIdx);
    setIspIdx(rTags, BPC2_LOWER_IDX, m_ISP_INT.sBpc2.u2LowerIdx);
    setIspTags2<ISP_NVRAM_BNR_BPC_T, ISP_MGR_BNR2_T, DIP_X_BPC2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // NR12
    CAM_LOGD_IF(m_bDebugEnable,"NR12:");
    setIspIdx(rTags, IDX_NR12, idxmgr.getIdx_NR1());
    setIspIdx(rTags, NR12_UPPER_IDX, m_ISP_INT.sNr12.u2UpperIdx);
    setIspIdx(rTags, NR12_LOWER_IDX, m_ISP_INT.sNr12.u2LowerIdx);
    setIspTags2<ISP_NVRAM_BNR_NR1_T, ISP_MGR_BNR2_T, DIP_X_NR12_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // PDC2
    CAM_LOGD_IF(m_bDebugEnable,"PDC2:");
    setIspIdx(rTags, IDX_PDC2, idxmgr.getIdx_PDC());
    setIspIdx(rTags, PDC2_UPPER_IDX, m_ISP_INT.sPdc2.u2UpperIdx);
    setIspIdx(rTags, PDC2_LOWER_IDX, m_ISP_INT.sPdc2.u2LowerIdx);
    setIspTags2<ISP_NVRAM_BNR_PDC_T, ISP_MGR_BNR2_T, DIP_X_PDC2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // RMM2
    CAM_LOGD_IF(m_bDebugEnable,"RMM2:");
    setIspIdx(rTags, IDX_RMM2, idxmgr.getIdx_RMM());
    setIspIdx(rTags, RMM2_UPPER_IDX, m_ISP_INT.sRmm2.u2UpperIdx);
    setIspIdx(rTags, RMM2_LOWER_IDX, m_ISP_INT.sRmm2.u2LowerIdx);
    setIspTags2<ISP_NVRAM_RMM_GET_T, ISP_MGR_RMM2_T, DIP_X_RMM2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // RMG2
    CAM_LOGD_IF(m_bDebugEnable,"RMG2:");
    setIspTags2<ISP_NVRAM_RMG_T, ISP_MGR_RMG2_T, DIP_X_RMG2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // LSC2
    CAM_LOGD_IF(m_bDebugEnable,"LSC2:");
    setIspTags2<ISP_NVRAM_LSC_T, ISP_MGR_LSC2_T, DIP_X_LSC2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // PGN
    CAM_LOGD_IF(m_bDebugEnable,"PGN:");
    setIspTags2<ISP_NVRAM_PGN_T, ISP_MGR_PGN_T, DIP_X_PGN_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // SL2
    CAM_LOGD_IF(m_bDebugEnable,"SL2:");
    setIspIdx(rTags, IDX_SL2, idxmgr.getIdx_SL2());
    setIspTags2<ISP_NVRAM_SL2_GET_T,  ISP_MGR_SL2_T, DIP_X_SL2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
    setIspTags2<ISP_NVRAM_SL2B_GET_T, ISP_MGR_SL2_T, DIP_X_SL2B_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
    setIspTags2<ISP_NVRAM_SL2C_GET_T, ISP_MGR_SL2_T, DIP_X_SL2C_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
    setIspTags2<ISP_NVRAM_SL2D_GET_T, ISP_MGR_SL2_T, DIP_X_SL2D_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
    setIspTags2<ISP_NVRAM_SL2H_GET_T, ISP_MGR_SL2_T, DIP_X_SL2H_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
    setIspTags2<ISP_NVRAM_SL2I_GET_T, ISP_MGR_SL2_T, DIP_X_SL2I_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // RNR
    CAM_LOGD_IF(m_bDebugEnable,"RNR:");
    setIspIdx(rTags, IDX_RNR, idxmgr.getIdx_RNR());
    setIspIdx(rTags, RNR_UPPER_IDX, m_ISP_INT.sRnr.u2UpperIdx);
    setIspIdx(rTags, RNR_LOWER_IDX, m_ISP_INT.sRnr.u2LowerIdx);
    setIspTags2<ISP_NVRAM_RNR_T, ISP_MGR_RNR_T, DIP_X_RNR_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // UDM
    CAM_LOGD_IF(m_bDebugEnable,"UDM:");
    setIspIdx(rTags, IDX_UDM, idxmgr.getIdx_UDM());
    setIspIdx(rTags, UDM_UPPER_IDX, m_ISP_INT.sUdm.u2UpperIdx);
    setIspIdx(rTags, UDM_LOWER_IDX, m_ISP_INT.sUdm.u2LowerIdx);
    setIspTags2<ISP_NVRAM_UDM_T, ISP_MGR_UDM_T, DIP_X_UDM_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // G2G
    CAM_LOGD_IF(m_bDebugEnable,"G2G:");
    setIspTags2<ISP_NVRAM_CCM_T, ISP_MGR_CCM_T, DIP_X_G2G_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
    ISP_CCM_WEIGHT_T CCM_Weight = ISP_MGR_CCM_T::getInstance(m_eSensorDev).getCCMWeight();
    setIspIdx(rTags, SMOOTH_CCM, m_rIspParam.bInvokeSmoothCCM);
    setIspIdx(rTags, CCM_Weight_Strobe, CCM_Weight.i4Strobe);
    setIspIdx(rTags, CCM_Weight_A, CCM_Weight.i4A);
    setIspIdx(rTags, CCM_Weight_TL84, CCM_Weight.i4TL84);
    setIspIdx(rTags, CCM_Weight_CWF, CCM_Weight.i4CWF);
    setIspIdx(rTags, CCM_Weight_D65, CCM_Weight.i4D65);
    setIspIdx(rTags, CCM_Weight_RSV1, CCM_Weight.i4RSV1);
    setIspIdx(rTags, CCM_Weight_RSV2, CCM_Weight.i4RSV2);
    setIspIdx(rTags, CCM_Weight_RSV3, CCM_Weight.i4RSV3);

/*
    // G2G_CFC
    MY_LOG_IF(m_bDebugEnable,"G2G_CFC:");
    setIspIdx(rTags, IDX_CCM_CFC, idxmgr.getIdx_CCM_CFC());
    setIspTags2<ISP_NVRAM_CCM_CTL_T, ISP_MGR_CCM_T, DIP_X_G2G_CFC_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
*/


    // GGM
    setIspIdx(rTags, IDX_GGM, idxmgr.getIdx_GGM());
    rDebugInfo.GGMTableInfo.u4TableSize = 288;
    ISP_NVRAM_GGM_T& rGGM = *(reinterpret_cast<ISP_NVRAM_GGM_T*>(rDebugInfo.GGMTableInfo.GGM));
    ISP_MGR_GGM_T::getInstance(m_eSensorDev).get(rGGM, static_cast<const dip_x_reg_t*>(pRegBuf));

    // G2C
    CAM_LOGD_IF(m_bDebugEnable,"G2C:");
    setIspTags2<ISP_NVRAM_G2C_T, ISP_MGR_G2C_T, DIP_X_G2C_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // ANR
    CAM_LOGD_IF(m_bDebugEnable,"ANR:");
    setIspIdx(rTags, IDX_ANR, idxmgr.getIdx_ANR());
    setIspIdx(rTags, ANR_UPPER_IDX, m_ISP_INT.sAnr.u2UpperIdx);
    setIspIdx(rTags, ANR_LOWER_IDX, m_ISP_INT.sAnr.u2LowerIdx);
    setIspTags2<ISP_NVRAM_ANR_T, ISP_MGR_NBC_T, DIP_X_ANR_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    rDebugInfo.ANRTableInfo.u4TableSize = 256;
    //ISP_MGR_NBC_T::getInstance(m_eSensorDev).getANR_TBL(rDebugInfo.ANRTableInfo.ANR);
    ISP_NVRAM_ANR_LUT_T& rANR_TBL = *(reinterpret_cast<ISP_NVRAM_ANR_LUT_T*>(rDebugInfo.ANRTableInfo.ANR));
    ISP_MGR_NBC_T::getInstance(m_eSensorDev).getANR_TBL(rANR_TBL, static_cast<const dip_x_reg_t*>(pRegBuf));

    // ANR2
    CAM_LOGD_IF(m_bDebugEnable,"ANR2:");
    setIspIdx(rTags, IDX_ANR2, idxmgr.getIdx_ANR2());
    setIspIdx(rTags, ANR2_UPPER_IDX, m_ISP_INT.sAnr2.u2UpperIdx);
    setIspIdx(rTags, ANR2_LOWER_IDX, m_ISP_INT.sAnr2.u2LowerIdx);
    setIspTags2<ISP_NVRAM_ANR2_T, ISP_MGR_NBC2_T, DIP_X_ANR2_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // CCR
    CAM_LOGD_IF(m_bDebugEnable,"CCR:");
    setIspIdx(rTags, IDX_CCR, idxmgr.getIdx_CCR());
    setIspIdx(rTags, CCR_UPPER_IDX, m_ISP_INT.sCcr.u2UpperIdx);
    setIspIdx(rTags, CCR_LOWER_IDX, m_ISP_INT.sCcr.u2LowerIdx);
    setIspTags2<ISP_NVRAM_CCR_T, ISP_MGR_NBC2_T, DIP_X_CCR_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

/*
    // BOK
    CAM_LOGD_IF(m_bDebugEnable,"BOK:");
    setIspIdx(rTags, IDX_BOK, idxmgr.getIdx_BOK());
    setIspTags2<ISP_NVRAM_BOK_T, ISP_MGR_NBC2_T, DIP_X_BOK_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
*/
    // PCA
    CAM_LOGD_IF(m_bDebugEnable,"PCA:");
    setIspIdx(rTags, IDX_PCA, m_pPcaMgr->getIdx());
    setIspIdx(rTags, PCA_SLIDER, m_pPcaMgr->getSliderValue());
    setIspTags2<ISP_NVRAM_PCA_T, ISP_MGR_PCA_T, DIP_X_PCA_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    rDebugInfo.PCATableInfo.u4TableSize = 180;
    ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).getLut(rDebugInfo.PCATableInfo.PCA);

    // HFG
    CAM_LOGD_IF(m_bDebugEnable,"HFG:");
    setIspIdx(rTags, IDX_HFG, idxmgr.getIdx_HFG());
    setIspIdx(rTags, HFG_UPPER_IDX, m_ISP_INT.sHfg.u2UpperIdx);
    setIspIdx(rTags, HFG_LOWER_IDX, m_ISP_INT.sHfg.u2LowerIdx);
    setIspTags2<ISP_NVRAM_HFG_GET_T, ISP_MGR_HFG_T, DIP_X_HFG_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    //  EE
    CAM_LOGD_IF(m_bDebugEnable,"EE:");
    setIspIdx(rTags, IDX_EE, idxmgr.getIdx_EE());
    setIspIdx(rTags, EE_UPPER_IDX, m_ISP_INT.sEe.u2UpperIdx);
    setIspIdx(rTags, EE_LOWER_IDX, m_ISP_INT.sEe.u2LowerIdx);
    setIspTags2<ISP_NVRAM_EE_T, ISP_MGR_SEEE_T, DIP_X_EE_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // SE
    CAM_LOGD_IF(m_bDebugEnable,"SE:");
    setIspTags2<ISP_NVRAM_SE_T, ISP_MGR_SEEE_T, DIP_X_SE_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

/*
    // NR3D
    CAM_LOGD_IF(m_bDebugEnable,"NR3D:");
    setIspIdx(rTags, IDX_NR3D, idxmgr.getIdx_NR3D());
    setIspIdx(rTags, NR3D_UPPER_ISO, m_ISP_INT.sNr3d.u4UpperISO);
    setIspIdx(rTags, NR3D_LOWER_ISO, m_ISP_INT.sNr3d.u4LowerISO);
    setIspIdx(rTags, NR3D_UPPER_IDX, m_ISP_INT.sNr3d.u2UpperIdx);
    setIspIdx(rTags, NR3D_LOWER_IDX, m_ISP_INT.sNr3d.u2LowerIdx);
    setIspTags2<ISP_NVRAM_NR3D_T, ISP_MGR_NR3D_T, DIP_X_NR3D_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);
*/
    // MFB
    CAM_LOGD_IF(m_bDebugEnable,"MFB:");
    setIspIdx(rTags, IDX_MFB, idxmgr.getIdx_MFB());
    setIspTags2<ISP_NVRAM_MFB_T, ISP_MGR_MFB_T, DIP_X_MFB_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // MIXER3
    CAM_LOGD_IF(m_bDebugEnable,"MIXER3:");
    setIspIdx(rTags, IDX_MIX3, idxmgr.getIdx_MIX3());
    setIspTags2<ISP_NVRAM_MIXER3_T, ISP_MGR_MIXER3_T, DIP_X_MIXER3_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    // LCE
    CAM_LOGD_IF(m_bDebugEnable,"LCE:");
    setIspTags2<ISP_NVRAM_LCE_T, ISP_MGR_LCE_T, DIP_X_LCE_Begin>(m_eSensorDev, static_cast<const dip_x_reg_t*>(pRegBuf), rTags);

    //dynamic Gamma
    // adaptive Gamma
    CAM_LOGD_IF(m_bDebugEnable,"adaptive Gamma:");
    setIspIdx(rTags, DIP_X_GMA_GMAMode, m_GmaExifInfo.i4GMAMode);
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
    setIspIdx(rTags, SensorMode, rCamInfo.eSensorMode);
    setIspIdx(rTags, SceneIdx, rCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOValue, rCamInfo.u4ISOValue);
    setIspIdx(rTags, ISOIdx, rCamInfo.eIdx_ISO);
    setIspIdx(rTags, UPPER_ISO_IDX, rCamInfo.eIdx_ISO_U);
    setIspIdx(rTags, LOWER_ISO_IDX, rCamInfo.eIdx_ISO_L);
    setIspIdx(rTags, ShadingIdx, rCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, ZoomRatio_x100, rCamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, LightValue_x10, rCamInfo.i4LightValue_x10);
    setIspIdx(rTags, SwnrEncEnableIsoThreshold, m_u4SwnrEncEnableIsoThreshold);
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
            rCamInfo.eIspProfile, rCamInfo.eSensorMode, rCamInfo.eIdx_Scene, rCamInfo.eIdx_ISO);
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
    addPair(rDebugInfo, idx, M_ANR_UPPER_IDX, m_ISP_INT.sAnr.u2UpperIdx);
    addPair(rDebugInfo, idx, M_ANR_LOWER_IDX, m_ISP_INT.sAnr.u2LowerIdx);

    addPair(rDebugInfo, idx, M_IDX_ANR2, idxmgr.getIdx_ANR2());
    addPair(rDebugInfo, idx, M_ANR2_UPPER_IDX, m_ISP_INT.sAnr2.u2UpperIdx);
    addPair(rDebugInfo, idx, M_ANR2_LOWER_IDX, m_ISP_INT.sAnr2.u2LowerIdx);

    addPair(rDebugInfo, idx, M_IDX_CCR, idxmgr.getIdx_CCR());
    addPair(rDebugInfo, idx, M_CCR_UPPER_IDX, m_ISP_INT.sCcr.u2UpperIdx);
    addPair(rDebugInfo, idx, M_CCR_LOWER_IDX, m_ISP_INT.sCcr.u2LowerIdx);

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


