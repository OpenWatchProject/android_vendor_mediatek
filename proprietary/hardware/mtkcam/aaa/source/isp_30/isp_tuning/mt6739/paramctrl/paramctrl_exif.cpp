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
#include <mtkcam/aaa/aaa_hal_common.h>
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
    ((MUINT32)offsetof(isp_reg_t, reg))

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
    const isp_reg_t* pReg,
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
    //  (2) Body.
    ////////////////////////////////////////////////////////////////////////////
    IspDebugTag (&rTags)[TagID_Total_Num] = m_rIspExifDebugInfo.debugInfo.tags;

    // ISP debug tag version
    setIspIdx(rTags, IspTagVersion, IspDebugTagVersion);


    ////////////////////////////////////////////////////////////////////////////
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////

    // DBS
    MY_LOG_IF(m_bDebugEnable,"DBS:");
    setIspIdx(rTags, IDX_DBS, m_IspNvramMgr.getIdx_DBS());
    setIspIdx(rTags, DBS_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4DbsUpperISO);
    setIspIdx(rTags, DBS_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4DbsLowerISO);
    setIspIdx(rTags, DBS_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2DbsUpperIdx);
    setIspIdx(rTags, DBS_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2DbsLowerIdx);

    // OBC
    MY_LOG_IF(m_bDebugEnable,"OBC:");
    setIspIdx(rTags, IDX_OBC, m_IspNvramMgr.getIdx_OBC());

    // BPC
    MY_LOG_IF(m_bDebugEnable,"BPC:");
    setIspIdx(rTags, IDX_BPC, m_IspNvramMgr.getIdx_BPC());
    setIspIdx(rTags, BPC_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4BpcUpperISO);
    setIspIdx(rTags, BPC_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4BpcLowerISO);
    setIspIdx(rTags, BPC_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2BpcUpperIdx);
    setIspIdx(rTags, BPC_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2BpcLowerIdx);

    // NR1
    MY_LOG_IF(m_bDebugEnable,"NR1:");
    setIspIdx(rTags, IDX_NR1, m_IspNvramMgr.getIdx_NR1());
    setIspIdx(rTags, NR1_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4Nr1UpperISO);
    setIspIdx(rTags, NR1_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4Nr1LowerISO);
    setIspIdx(rTags, NR1_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2Nr1UpperIdx);
    setIspIdx(rTags, NR1_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2Nr1LowerIdx);

    // PDC
    MY_LOG_IF(m_bDebugEnable,"PDC:");
    setIspIdx(rTags, IDX_PDC, m_IspNvramMgr.getIdx_PDC());
    setIspIdx(rTags, PDC_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4PdcUpperISO);
    setIspIdx(rTags, PDC_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4PdcLowerISO);
    setIspIdx(rTags, PDC_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2PdcUpperIdx);
    setIspIdx(rTags, PDC_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2PdcLowerIdx);

    // SL2
    MY_LOG_IF(m_bDebugEnable,"NSL2:");
    setIspIdx(rTags, IDX_NSL2A, m_IspNvramMgr.getIdx_SL2());

    // CFA
    MY_LOG_IF(m_bDebugEnable,"CFA:");
    setIspIdx(rTags, IDX_CFA, m_IspNvramMgr.getIdx_CFA());
    setIspIdx(rTags, CFA_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4CfaUpperISO);
    setIspIdx(rTags, CFA_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4CfaLowerISO);
    setIspIdx(rTags, CFA_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2CfaUpperIdx);
    setIspIdx(rTags, CFA_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2CfaLowerIdx);


    // GGM
    //setIspIdx(rTags, IDX_GGM, m_IspNvramMgr.getIdx_GGM());

    /*m_rIspExifDebugInfo.GGMTableInfo.u4TableSize = 288;
    ISP_NVRAM_GGM_T& rGGM = *(reinterpret_cast<ISP_NVRAM_GGM_T*>(m_rIspExifDebugInfo.GGMTableInfo.GGM));
    ISP_MGR_GGM_T::getInstance(getSensorDev()).get(rGGM);*/

    // ANR
    MY_LOG_IF(m_bDebugEnable,"ANR:");
    setIspIdx(rTags, IDX_ANR, m_IspNvramMgr.getIdx_ANR());
    setIspIdx(rTags, ANR_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4AnrUpperISO);
    setIspIdx(rTags, ANR_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4AnrLowerISO);
    setIspIdx(rTags, ANR_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2AnrUpperIdx);
    setIspIdx(rTags, ANR_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2AnrLowerIdx);

    // ANR_TBL
    setIspIdx(rTags, ANR_TBL_CT_IDX, m_rIspCamInfo.eIdx_ANR_TBL_CT);
    setIspIdx(rTags, ANR_TBL_ISO_IDX, m_rIspCamInfo.eIdx_ANR_TBL_ISO);

    // CCR
    MY_LOG_IF(m_bDebugEnable,"CCR:");
    setIspIdx(rTags, IDX_CCR, m_IspNvramMgr.getIdx_CCR());

    // PCA
    MY_LOG_IF(m_bDebugEnable,"PCA:");
    setIspIdx(rTags, IDX_PCA, m_pPcaMgr->getIdx());
    //setIspIdx(rTags, PCA_SLIDER, m_pPcaMgr->getSliderValue());

    /*m_rIspExifDebugInfo.PCATableInfo.u4TableSize = 180;
    ISP_MGR_PCA_T::getInstance(m_eSensorDev, m_ePCAMode).getLut(m_rIspExifDebugInfo.PCATableInfo.PCA);*/

    //  EE
    MY_LOG_IF(m_bDebugEnable,"EE:");
    setIspIdx(rTags, IDX_EE, m_IspNvramMgr.getIdx_EE());
    setIspIdx(rTags, EE_UPPER_ISO, m_rIspCamInfo.rIspIntInfo.u4EeUpperISO);
    setIspIdx(rTags, EE_LOWER_ISO, m_rIspCamInfo.rIspIntInfo.u4EeLowerISO);
    setIspIdx(rTags, EE_UPPER_IDX, m_rIspCamInfo.rIspIntInfo.u2EeUpperIdx);
    setIspIdx(rTags, EE_LOWER_IDX, m_rIspCamInfo.rIspIntInfo.u2EeLowerIdx);

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
    MY_LOG_IF(m_bDebugEnable,"RAWIspCamInfo:");
    setIspIdx(rTags, IspProfile, m_rIspCamInfo.eIspProfile);
    setIspIdx(rTags, SensorMode, m_rIspCamInfo.eSensorMode);
    setIspIdx(rTags, SceneIdx, m_rIspCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOValue, m_rIspCamInfo.u4ISOValue);
    setIspIdx(rTags, ISOIdx, m_rIspCamInfo.eIdx_ISO);
    setIspIdx(rTags, ShadingIdx, m_rIspCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, ZoomRatio_x100, m_rIspCamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, LightValue_x10, m_rIspCamInfo.i4LightValue_x10);
    setIspIdx(rTags, SwnrEncEnableIsoThreshold, m_u4SwnrEncEnableIsoThreshold);
    //
    //  (2.4) EffectMode
    MY_LOG_IF(m_bDebugEnable,"EffectMode:");
    setIspIdx(rTags, EffectMode, getEffect());
    //
    //  (2.5) UserSelectLevel
    MY_LOG_IF(m_bDebugEnable,"UserSelectLevel:");
    setIspIdx(rTags, EdgeIdx, getIspUsrSelectLevel().eIdx_Edge);
    setIspIdx(rTags, HueIdx, getIspUsrSelectLevel().eIdx_Hue);
    setIspIdx(rTags, SatIdx, getIspUsrSelectLevel().eIdx_Sat);
    setIspIdx(rTags, BrightIdx, getIspUsrSelectLevel().eIdx_Bright);
    setIspIdx(rTags, ContrastIdx, getIspUsrSelectLevel().eIdx_Contrast);
    //
    //  (2.6) Output
    rDebugInfo.debugInfo = m_rIspExifDebugInfo.debugInfo;
    //rDebugInfo.GGMTableInfo = m_rIspExifDebugInfo.GGMTableInfo;
    //rDebugInfo.PCATableInfo = m_rIspExifDebugInfo.PCATableInfo;

    //  (3) Reset to the default.
    ::memset(&m_rIspExifDebugInfo, 0, sizeof(m_rIspExifDebugInfo));

    MUINT32 u4RegCnt = (_A_P1(CAM_LSCI_D_CON3) - _A_P1(CAM_CTL_START)) / 4 + 1;
    MUINT32 u4RRZOfst = (_A_P1(CAM_RRZ_CTL) - _A_P1(CAM_CTL_START)) / 4; //start address of rrz register
    MVOID* pP1TuningBuffer = IspTuningBufCtrl::getInstance(m_eSensorDev)->getP1Buffer();

    if(u4RegCnt > P1_BUFFER_SIZE)
    {
        CAM_LOGE("P1 buffer:(%d) byte exceed EXIF allocation size:(%d) byte", u4RegCnt*4, P1_BUFFER_SIZE*4);
    }
    else
    {
        if (pP1TuningBuffer != NULL)
        {
            memcpy((((MUINT8 *)rDebugInfo.P1RegInfo.regData)), ((MUINT8 *)pP1TuningBuffer + _A_P1(CAM_CTL_START)), P1_BUFFER_SIZE*4);
            memcpy(((MUINT8 *)rDebugInfo.P1RegInfo.regData) +u4RRZOfst, &m_RRZInfo, sizeof(ISP_NVRAM_RRZ_T)); //overwrite rrz register by member
            rDebugInfo.P1RegInfo.u4TableSize = P1_BUFFER_SIZE+1;
            rDebugInfo.P1RegInfo.u4HwVersion = 7;
            rDebugInfo.P1RegInfo.u4Offset = _A_P1(CAM_CTL_START);
        }
    }

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
    MY_LOG_IF(m_bDebugEnable,"CAM_CTL_EN_P2:");

    // SL2G
    /*MY_LOG_IF(m_bDebugEnable,"SL2:");
    setIspIdx(rTags, IDX_SL2G, m_ParamIdx_P2.SL2);*/

    // DBS2
    MY_LOG_IF(m_bDebugEnable,"DBS2:");
    setIspIdx(rTags, IDX_DBS, m_ParamIdx_P2.DBS);
    setIspIdx(rTags, DBS_UPPER_ISO, m_ISP_INT.u4DbsUpperISO);
    setIspIdx(rTags, DBS_LOWER_ISO, m_ISP_INT.u4DbsLowerISO);
    setIspIdx(rTags, DBS_UPPER_IDX, m_ISP_INT.u2DbsUpperIdx);
    setIspIdx(rTags, DBS_LOWER_IDX, m_ISP_INT.u2DbsLowerIdx);

    // OBC2
    MY_LOG_IF(m_bDebugEnable,"OBC2:");
    setIspIdx(rTags, IDX_OBC, m_ParamIdx_P2.OBC);

    // BPC2
    MY_LOG_IF(m_bDebugEnable,"BPC2:");
    setIspIdx(rTags, IDX_BPC, m_ParamIdx_P2.BNR_BPC);
    setIspIdx(rTags, BPC_UPPER_ISO, m_ISP_INT.u4BpcUpperISO);
    setIspIdx(rTags, BPC_LOWER_ISO, m_ISP_INT.u4BpcLowerISO);
    setIspIdx(rTags, BPC_UPPER_IDX, m_ISP_INT.u2BpcUpperIdx);
    setIspIdx(rTags, BPC_LOWER_IDX, m_ISP_INT.u2BpcLowerIdx);

    // NR12
    MY_LOG_IF(m_bDebugEnable,"NR12:");
    setIspIdx(rTags, IDX_NR1, m_ParamIdx_P2.BNR_NR1);
    setIspIdx(rTags, NR1_UPPER_ISO, m_ISP_INT.u4Nr1UpperISO);
    setIspIdx(rTags, NR1_LOWER_ISO, m_ISP_INT.u4Nr1LowerISO);
    setIspIdx(rTags, NR1_UPPER_IDX, m_ISP_INT.u2Nr1UpperIdx);
    setIspIdx(rTags, NR1_LOWER_IDX, m_ISP_INT.u2Nr1LowerIdx);

    // PDC2
    MY_LOG_IF(m_bDebugEnable,"PDC2:");
    setIspIdx(rTags, IDX_PDC, m_ParamIdx_P2.BNR_PDC);
    setIspIdx(rTags, PDC_UPPER_ISO, m_ISP_INT.u4PdcUpperISO);
    setIspIdx(rTags, PDC_LOWER_ISO, m_ISP_INT.u4PdcLowerISO);
    setIspIdx(rTags, PDC_UPPER_IDX, m_ISP_INT.u2PdcUpperIdx);
    setIspIdx(rTags, PDC_LOWER_IDX, m_ISP_INT.u2PdcLowerIdx);

    // CFA
    MY_LOG_IF(m_bDebugEnable,"CFA:");
    setIspIdx(rTags, IDX_CFA, m_ParamIdx_P2.CFA);
    setIspIdx(rTags, CFA_UPPER_ISO, m_ISP_INT.u4CfaUpperISO);
    setIspIdx(rTags, CFA_LOWER_ISO, m_ISP_INT.u4CfaLowerISO);
    setIspIdx(rTags, CFA_UPPER_IDX, m_ISP_INT.u2CfaUpperIdx);
    setIspIdx(rTags, CFA_LOWER_IDX, m_ISP_INT.u2CfaLowerIdx);

    // ANR
    MY_LOG_IF(m_bDebugEnable,"ANR:");
    setIspIdx(rTags, IDX_ANR, m_ParamIdx_P2.ANR);
    setIspIdx(rTags, ANR_UPPER_ISO, m_ISP_INT.u4AnrUpperISO);
    setIspIdx(rTags, ANR_LOWER_ISO, m_ISP_INT.u4AnrLowerISO);
    setIspIdx(rTags, ANR_UPPER_IDX, m_ISP_INT.u2AnrUpperIdx);
    setIspIdx(rTags, ANR_LOWER_IDX, m_ISP_INT.u2AnrLowerIdx);

    // ANR_TBL
    setIspIdx(rTags, ANR_TBL_CT_IDX, rCamInfo.eIdx_ANR_TBL_CT);
    setIspIdx(rTags, ANR_TBL_ISO_IDX, rCamInfo.eIdx_ANR_TBL_ISO);

    // CCR
    MY_LOG_IF(m_bDebugEnable,"CCR:");
    setIspIdx(rTags, IDX_CCR, m_ParamIdx_P2.CCR);

    //  EE
    MY_LOG_IF(m_bDebugEnable,"EE:");
    setIspIdx(rTags, IDX_EE, m_ParamIdx_P2.EE);
    setIspIdx(rTags, EE_UPPER_ISO, m_ISP_INT.u4EeUpperISO);
    setIspIdx(rTags, EE_LOWER_ISO, m_ISP_INT.u4EeLowerISO);
    setIspIdx(rTags, EE_UPPER_IDX, m_ISP_INT.u2EeUpperIdx);
    setIspIdx(rTags, EE_LOWER_IDX, m_ISP_INT.u2EeLowerIdx);


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
    MY_LOG_IF(m_bDebugEnable,"RAWIspCamInfo:");
    setIspIdx(rTags, IspProfile, rCamInfo.eIspProfile);
    setIspIdx(rTags, SensorMode, rCamInfo.eSensorMode);
    setIspIdx(rTags, SceneIdx, rCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOValue, rCamInfo.u4ISOValue);
    setIspIdx(rTags, ISOIdx, rCamInfo.eIdx_ISO);
    setIspIdx(rTags, ShadingIdx, rCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, ZoomRatio_x100, rCamInfo.i4ZoomRatio_x100);
    setIspIdx(rTags, LightValue_x10, rCamInfo.i4LightValue_x10);

    //
    //  (2.4) EffectMode
    MY_LOG_IF(m_bDebugEnable,"EffectMode:");
    setIspIdx(rTags, EffectMode, rCamInfo.eIdx_Effect);
    //
    //  (2.5) UserSelectLevel
    MY_LOG_IF(m_bDebugEnable,"UserSelectLevel:");
    setIspIdx(rTags, EdgeIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Edge);
    setIspIdx(rTags, HueIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Hue);
    setIspIdx(rTags, SatIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Sat);
    setIspIdx(rTags, BrightIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Bright);
    setIspIdx(rTags, ContrastIdx, rCamInfo.rIspUsrSelectLevel.eIdx_Contrast);


    MUINT32 p2Size = (_A_P1(CAM_CCR_HUE3) - _A_P1(CAM_CTL_START)) / 4 + 1;
    if (p2Size/4 > P2_BUFFER_SIZE)
    {
        CAM_LOGE("P2 buffer:(%d) byte exceed EXIF allocation size:(%d) byte", p2Size, P2_BUFFER_SIZE*4);
    }
    else
    {
        memcpy(rDebugInfo.P2RegInfo.regDataP2, (UINT8*)pRegBuf+_A_P1(CAM_CTL_START), p2Size*4);
        rDebugInfo.P2RegInfo.u4TableSize = P2_BUFFER_SIZE+1;
        rDebugInfo.P2RegInfo.u4HwVersion = 7;
        rDebugInfo.P2RegInfo.u4Offset = _A_P1(CAM_NSL2A_SIZE);
    }

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
    /*addPair(rDebugInfo, idx, M_IspProfile, rCamInfo.eIspProfile);

    addPair(rDebugInfo, idx, M_IDX_ANR, idxmgr.getIdx_ANR());
    addPair(rDebugInfo, idx, M_ANR_UPPER_IDX, m_ISP_INT.sAnr.u2UpperIso_LowerZoom);
    addPair(rDebugInfo, idx, M_ANR_LOWER_IDX, m_ISP_INT.sAnr.u2LowerIso_LowerZoom);

    addPair(rDebugInfo, idx, M_IDX_ANR2, idxmgr.getIdx_ANR2());
    addPair(rDebugInfo, idx, M_ANR2_UPPER_IDX, m_ISP_INT.sAnr2.u2UpperIso_LowerZoom);
    addPair(rDebugInfo, idx, M_ANR2_LOWER_IDX, m_ISP_INT.sAnr2.u2LowerIso_LowerZoom);

    addPair(rDebugInfo, idx, M_IDX_CCR, idxmgr.getIdx_CCR());
    addPair(rDebugInfo, idx, M_CCR_UPPER_IDX, m_ISP_INT.sCcr.u2UpperIso_LowerZoom);
    addPair(rDebugInfo, idx, M_CCR_LOWER_IDX, m_ISP_INT.sCcr.u2LowerIso_LowerZoom);*/

    // ISP Top control
    /*ISP_NVRAM_CTL_EN_P2_T rCTL;
    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).get(rCTL, static_cast<const isp_reg_t*>(pRegBuf));
    addPair(rDebugInfo, idx, M_DIP_X_CTL_YUV_EN, rCTL.en_yuv.val);

    // ISP ANR registers
    ISP_NVRAM_ANR_T rANR;
    ISP_MGR_NBC_T::getInstance(m_eSensorDev).get(rANR, static_cast<const isp_reg_t*>(pRegBuf));
    int anr_begin = M_DIP_X_ANR_CON1;
    int anrSize = ISP_NVRAM_ANR_T::COUNT;
    for (int i=0; i<anrSize; i++)
        addPair(rDebugInfo, idx, anr_begin + i, rANR.set[i]);

    // ISP ANR2 registers
    ISP_NVRAM_ANR2_T rANR2;
    ISP_MGR_NBC2_T::getInstance(m_eSensorDev).get(rANR2, static_cast<const isp_reg_t*>(pRegBuf));
    int anr2_begin = M_DIP_X_ANR2_CON1;
    int anr2Size = ISP_NVRAM_ANR2_T::COUNT;
    for (int i=0; i<anr2Size; i++)
        addPair(rDebugInfo, idx, anr2_begin + i, rANR2.set[i]);

    // ISP CCR registers
    ISP_NVRAM_CCR_T rCCR;
    ISP_MGR_NBC2_T::getInstance(m_eSensorDev).get(rCCR, static_cast<const isp_reg_t*>(pRegBuf));
    int ccr_begin = M_DIP_X_CCR_CON;
    int ccrSize = ISP_NVRAM_CCR_T::COUNT;
    for (int i=0; i<ccrSize; i++)
        addPair(rDebugInfo, idx, ccr_begin + i, rCCR.set[i]);
*/

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setDebugInfo4TG(MUINT32 const u4Rto, MINT32 const OBCGain, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    MUINT32 u4OBCGainOffset;
    MUINT32 u4RtoOffset;
    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN0))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN1))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN2))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN3))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCGain;

    u4RtoOffset = ((MUINT32)offsetof(isp_reg_t, CAM_LSC_RATIO))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

    return  MERR_OK;
}

MERROR_ENUM
Paramctrl::
setDebugInfo4CCU(MUINT32 const u4Rto, ISP_NVRAM_OBC_T const &OBCResult, NSIspExifDebug::IspExifDebugInfo_T& rDbgIspInfo)
{
    MUINT32 u4OBCGainOffset;
    MUINT32 u4OBCOFFSTOffset;
    MUINT32 u4RtoOffset;

    // OBC Gain
    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN0))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain0.val;

    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN1))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain1.val;

    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN2))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain2.val;

    u4OBCGainOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_GAIN3))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCGainOffset) = OBCResult.gain3.val;

    // OBC OFFST
    u4OBCOFFSTOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_OFFST0))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) = OBCResult.offst0.val;

    u4OBCOFFSTOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_OFFST1))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) =  OBCResult.offst1.val;

    u4OBCOFFSTOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_OFFST2))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) = OBCResult.offst2.val;

    u4OBCOFFSTOffset = ((MUINT32)offsetof(isp_reg_t, CAM_OBC_OFFST3))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4OBCOFFSTOffset) = OBCResult.offst3.val;

    // Ratio
    u4RtoOffset = ((MUINT32)offsetof(isp_reg_t, CAM_LSC_RATIO))/4 - _A_P1(CAM_CTL_START)/4;
    *(rDbgIspInfo.P1RegInfo.regData+u4RtoOffset) = (u4Rto<<24)|(u4Rto<<16)|(u4Rto<<8)|u4Rto;

    return  MERR_OK;
}

