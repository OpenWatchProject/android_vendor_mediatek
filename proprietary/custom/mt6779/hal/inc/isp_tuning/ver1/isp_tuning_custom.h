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
#ifndef _ISP_TUNING_CUSTOM_H_
#define _ISP_TUNING_CUSTOM_H_

#include <debug_exif/aaa/dbg_isp_param.h>
#include <tuning/shading_flow_custom.h>

namespace NSIspTuning
{

typedef MUINT64 ISP_FEATURE_TOP_CTL;
typedef enum {
    F_OBC = 0,
    F_LSC,
    F_BPC,
    F_CT,
    F_PDC,
    F_DGN,
    F_WB,
    F_HLR,
    F_LTM,
    F_SLK,
    F_DM,  //10
    F_LDNR,
    F_FLC,
    F_CCM,
    F_GGM,
    F_YNR,
    F_LCE,
    F_DCE,
    F_EE,
    F_CNR,
    F_CCR, //20
    F_ABF,
    F_BOK,
    F_COLOR,
    F_MIX,
    F_MFB, //25
    F_LPCNR,
    F_YNR_LINK,
    F_NDG
}eISP_FEATURE_TOP_CONTROL;

#define F_OBC_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_OBC)  //0
#define F_LSC_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_LSC)
#define F_BPC_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_BPC)
#define F_CT_EN(fgOnOff)     ((MUINT64)(fgOnOff) << F_CT)
#define F_PDC_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_PDC)
#define F_DGN_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_DGN)
#define F_WB_EN(fgOnOff)     ((MUINT64)(fgOnOff) << F_WB)
#define F_HLR_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_HLR)
#define F_LTM_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_LTM)
#define F_SLK_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_SLK)
#define F_DM_EN(fgOnOff)     ((MUINT64)(fgOnOff) << F_DM)   //10
#define F_LDNR_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_LDNR)
#define F_FLC_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_FLC)
#define F_CCM_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_CCM)
#define F_GGM_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_GGM)
#define F_YNR_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_YNR)
#define F_LCE_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_LCE)
#define F_DCE_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_DCE)
#define F_EE_EN(fgOnOff)     ((MUINT64)(fgOnOff) << F_EE)
#define F_CNR_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_CNR)
#define F_CCR_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_CCR)  //20
#define F_ABF_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_ABF)
#define F_BOK_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_BOK)
#define F_COLOR_EN(fgOnOff)  ((MUINT64)(fgOnOff) << F_COLOR)
#define F_MIX_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_MIX)
#define F_MFB_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_MFB)  //25
#define F_LPCNR_EN(fgOnOff)  ((MUINT64)(fgOnOff) << F_LPCNR)
#define F_YNR_LINK_EN(fgOnOff)  ((MUINT64)(fgOnOff) << F_YNR_LINK)
#define F_NDG_EN(fgOnOff)  ((MUINT64)(fgOnOff) << F_NDG)


#define M_OBC_EN    ((MUINT64)1 << F_OBC)  //0
#define M_LSC_EN    ((MUINT64)1 << F_LSC)
#define M_BPC_EN    ((MUINT64)1 << F_BPC)
#define M_CT_EN     ((MUINT64)1 << F_CT)
#define M_PDC_EN    ((MUINT64)1 << F_PDC)
#define M_DGN_EN    ((MUINT64)1 << F_DGN)
#define M_WB_EN     ((MUINT64)1 << F_WB)
#define M_HLR_EN    ((MUINT64)1 << F_HLR)
#define M_LTM_EN    ((MUINT64)1 << F_LTM)
#define M_SLK_EN    ((MUINT64)1 << F_SLK)
#define M_DM_EN     ((MUINT64)1 << F_DM)    //10
#define M_LDNR_EN   ((MUINT64)1 << F_LDNR)
#define M_FLC_EN    ((MUINT64)1 << F_FLC)
#define M_CCM_EN    ((MUINT64)1 << F_CCM)
#define M_GGM_EN    ((MUINT64)1 << F_GGM)
#define M_YNR_EN    ((MUINT64)1 << F_YNR)
#define M_LCE_EN    ((MUINT64)1 << F_LCE)
#define M_DCE_EN    ((MUINT64)1 << F_DCE)
#define M_EE_EN     ((MUINT64)1 << F_EE)
#define M_CNR_EN    ((MUINT64)1 << F_CNR)
#define M_CCR_EN    ((MUINT64)1 << F_CCR)   //20
#define M_ABF_EN    ((MUINT64)1 << F_ABF)
#define M_BOK_EN    ((MUINT64)1 << F_BOK)
#define M_COLOR_EN  ((MUINT64)1 << F_COLOR)
#define M_MIX_EN    ((MUINT64)1 << F_MIX)
#define M_MFB_EN    ((MUINT64)1 << F_MFB)   //25
#define M_LPCNR_EN  ((MUINT64)1 << F_LPCNR)
#define M_YNR_LINK_EN  ((MUINT64)1 << F_YNR_LINK)
#define M_NDG_EN  ((MUINT64)1 << F_NDG)

/*******************************************************************************
*
*******************************************************************************/
class IspTuningCustom
{
public:  ////    Ctor/Dtor.
    IspTuningCustom()
    :m_idx_valid (MFALSE)
    ,m_CCM_DualPara_valid(MFALSE)
    {
        ::memset(&m_CCM_DualPara, 0, sizeof(MUINT32));
        ::memset(&m_Zoom_Env, 0, sizeof(ISP_NVRAM_LV_INTERVAL_T));
    }
    virtual ~IspTuningCustom() {}

    static IspTuningCustom* createInstance(MUINT32 const eSensorDev, MUINT32 const u4SensorID);
    virtual void destroyInstance() = 0;

private:
    MBOOL m_idx_valid;
    MUINT32 m_CCM_DualPara[3];
    MBOOL m_CCM_DualPara_valid;
    ISP_NVRAM_ZOOM_INTERVAL_T m_Zoom_Env;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual MUINT32   getSensorDev() const = 0;
    virtual MUINT32   getSensorID() const = 0;
    virtual MVOID refine_CamInfo(RAWIspCamInfo& rCamInfo);

public:     ////    Operations.

    template <class T>
	T LIMIT(T const value, T const low_bound, T const upper_bound)
    {
        if (value < low_bound){ return (low_bound);}
        else if (value > upper_bound){ return (upper_bound);}
		else{ return (value);}
    }

    virtual
    MVOID*
    get_feature_control(
        MUINT32 eSensorDev
    ) const;

    virtual
    MVOID
    refine_OBC(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_OBC_T& rOBC);

    virtual
    MVOID
    refine_BPC(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_BPC_BPC_T& rBPC);

    virtual
    MVOID
    refine_CT(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_BPC_CT_T& rCT);

    virtual
    MVOID
    refine_PDC(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_BPC_PDC_T& rPDC);

    virtual
    MVOID
    refine_DM(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DM_T& rDM);

    virtual
    MVOID
    refine_FLC(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_FLC_T& rFLC);

    virtual
    MVOID
    refine_CCM(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CCM_T& rCCM);

    virtual
    MVOID
    refine_GGM(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_GGM_T& rGGM);

    virtual
    MVOID
    refine_YNRS(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_YNRS_T& rYNRS);

    virtual
    MVOID
    refine_YNR(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_YNR_T& rYNR);

    virtual
    MVOID
    refine_LDNR(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_LDNR_T& rLDNR);

    virtual
    MVOID
    refine_LCE(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_LCE_T& rLCE);

    virtual
    MVOID
    refine_DCE(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DCE_T& rDCE);

    virtual
    MVOID
    refine_EE(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_EE_T& rEE);

    virtual
    MVOID
    refine_CNR(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CNR_CNR_T& rCNR);

    virtual
    MVOID
    refine_CCR(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CNR_CCR_T& rCCR);

    virtual
    MVOID
    refine_ABF(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CNR_ABF_T& rABF);

    virtual
    MVOID
    refine_COLOR(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_COLOR_T& rCOLOR);

    virtual
    MVOID
    refine_MIX(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_MIX_T& rMIX);

    virtual
    MVOID
    refine_MFB(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_MFB_T& rMFB);

    virtual
    MVOID
    refine_NR3D(
        MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR3D_T& rNR3D);

    virtual
    MVOID
    RGB2WMatrix(
        const MUINT32 L[], const AWB_GAIN_T& PGN, ISP_NVRAM_CCM_T& W);

    virtual
    MVOID
    RGB2YLinear(
        const ISP_NVRAM_CCM_T& inW, ISP_NVRAM_CCM_T& outW);

    virtual
    MBOOL
    is_to_invoke_capture_in_color(
        RAWIspCamInfo const& rCamInfo);

    virtual
    MBOOL
    is_to_invoke_isp_interpolation(
        RAWIspCamInfo const& rCamInfo);

    virtual
    MBOOL
    is_HDRTuning(
        RAWIspCamInfo const& rCamInfo);

    virtual
    MBOOL
    is_ZHDRTuning(
        RAWIspCamInfo const& rCamInfo);

    virtual
    MVOID
    userSetting_EE(EIndex_Isp_Edge_T eIdx_Edge, ISP_NVRAM_EE_T& rEE);

    virtual
    MVOID
    userSetting_HSBC(
        IspUsrSelectLevel_T const &rIspUsrSelectLevel, ISP_NVRAM_G2CX_T &rG2C);

    virtual
    EIndex_Shading_CCT_T
    evaluate_Shading_CCT_index  (LSC_INPUT_INFO_T const& rLSCInfo) const;

    virtual
    MINT32
    evaluate_Shading_Ratio  (LSC_INPUT_INFO_T const& rLSCInfo,
                             MBOOL isSuperNightMode,
                             MINT32 pParam
    );

private:
    int total_RA_num_frames_;// = 0;        // totol number of frames
    int ISO_Buffer_[10];

public:

    virtual
    MINT32
    map_AppName_to_MappingInfo(
        const char* pAppname
    );

    virtual
    MINT32
    map_Target_Size_to_MappingInfo(
        MUINT32 width, MUINT32 height, const char *pUserString);

    virtual
    MVOID
    set_nvram_value(const NVRAM_CAMERA_ISP_PARAM_STRUCT* param);

    virtual
    MINT32
    map_zoom_value_to_index(MINT32 i4ZoomRatio);

};


};  //  NSIspTuning
#endif //  _ISP_TUNING_CUSTOM_H_

