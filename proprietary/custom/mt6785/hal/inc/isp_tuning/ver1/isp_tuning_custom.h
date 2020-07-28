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

namespace NSIspTuning
{

typedef MUINT64 ISP_FEATURE_TOP_CTL;
typedef enum {
    F_SL2FG = 0,
    F_ADBS,
    F_DBS,
    F_OBC,
    F_BPC,
    F_NR1,
    F_RMM,
    F_RMG,
    F_RPGN,
    F_SL2H,
    F_RNR,    //10
    F_SL2,
    F_UDM,
    F_CCM,
    F_CCM2,
    F_LCE,
    F_GGM,
    F_GGM2,
    F_G2C,
    F_SL2B,
    F_ANR,    //20
    F_SL2C,
    F_ANR2,
    F_CCR,
    F_BOK,
    F_ABF,
    F_NDG,
    F_NDG2,
    F_PCA,
    F_SL2I,
    F_HFG,    //30
    F_SL2D,
    F_EE,
    F_SL2E,
    F_COLOR,
    F_MIXER3
}eISP_FEATURE_TOP_CONTROL;

#define F_SL2FG_EN(fgOnOff)  ((MUINT64)(fgOnOff) << F_SL2FG)  //0
#define F_DBS_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_DBS)
#define F_ADBS_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_ADBS)
#define F_OBC_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_OBC)
#define F_BPC_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_BPC)
#define F_NR1_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_NR1)
#define F_RMM_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_RMM)
#define F_RMG_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_RMG)
#define F_RPGN_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_RPGN)
#define F_SL2H_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_SL2H)
#define F_RNR_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_RNR)  //10
#define F_SL2_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_SL2)
#define F_UDM_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_UDM)
#define F_CCM_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_CCM)
#define F_CCM2_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_CCM2)
#define F_LCE_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_LCE)
#define F_GGM_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_GGM)
#define F_GGM2_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_GGM2)
#define F_G2C_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_G2C)
#define F_SL2B_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_SL2B)
#define F_ANR_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_ANR)   //20
#define F_SL2C_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_SL2C)
#define F_ANR2_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_ANR2)
#define F_CCR_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_CCR)
#define F_BOK_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_BOK)
#define F_ABF_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_ABF)
#define F_NDG_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_NDG)
#define F_NDG2_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_NDG2)
#define F_PCA_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_PCA)
#define F_SL2I_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_SL2I)
#define F_HFG_EN(fgOnOff)    ((MUINT64)(fgOnOff) << F_HFG)  //30
#define F_SL2D_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_SL2D)
#define F_EE_EN(fgOnOff)     ((MUINT64)(fgOnOff) << F_EE)
#define F_SL2E_EN(fgOnOff)   ((MUINT64)(fgOnOff) << F_SL2E)
#define F_COLOR_EN(fgOnOff)  ((MUINT64)(fgOnOff) << F_COLOR)
#define F_MIXER3_EN(fgOnOff) ((MUINT64)(fgOnOff) << F_MIXER3)


#define M_SL2FG_EN  ((MUINT64)1 << F_SL2FG)
#define M_DBS_EN    ((MUINT64)1 << F_DBS)
#define M_OBC_EN    ((MUINT64)1 << F_OBC)
#define M_ADBS_EN   ((MUINT64)1 << F_ADBS)
#define M_BPC_EN    ((MUINT64)1 << F_BPC)
#define M_NR1_EN    ((MUINT64)1 << F_NR1)
#define M_RMM_EN    ((MUINT64)1 << F_RMM)
#define M_RMG_EN    ((MUINT64)1 << F_RMG)
#define M_RPGN_EN   ((MUINT64)1 << F_RPGN)
#define M_SL2H_EN   ((MUINT64)1 << F_SL2H)  //10
#define M_RNR_EN    ((MUINT64)1 << F_RNR)
#define M_SL2_EN    ((MUINT64)1 << F_SL2)
#define M_UDM_EN    ((MUINT64)1 << F_UDM)
#define M_CCM_EN    ((MUINT64)1 << F_CCM)
#define M_CCM2_EN    ((MUINT64)1 << F_CCM2)
#define M_LCE_EN    ((MUINT64)1 << F_LCE)
#define M_GGM_EN    ((MUINT64)1 << F_GGM)
#define M_GGM2_EN    ((MUINT64)1 << F_GGM2)
#define M_G2C_EN    ((MUINT64)1 << F_G2C)
#define M_SL2B_EN   ((MUINT64)1 << F_SL2B)  //20
#define M_ANR_EN    ((MUINT64)1 << F_ANR)
#define M_SL2C_EN   ((MUINT64)1 << F_SL2C)
#define M_ANR2_EN   ((MUINT64)1 << F_ANR2)
#define M_CCR_EN    ((MUINT64)1 << F_CCR)
#define M_BOK_EN    ((MUINT64)1 << F_BOK)
#define M_ABF_EN    ((MUINT64)1 << F_ABF)
#define M_NDG_EN    ((MUINT64)1 << F_NDG)
#define M_NDG2_EN    ((MUINT64)1 << F_NDG2)
#define M_PCA_EN    ((MUINT64)1 << F_PCA)
#define M_SL2I_EN   ((MUINT64)1 << F_SL2I)  //30
#define M_HFG_EN    ((MUINT64)1 << F_HFG)
#define M_SL2D_EN   ((MUINT64)1 << F_SL2D)
#define M_EE_EN     ((MUINT64)1 << F_EE)
#define M_SL2E_EN   ((MUINT64)1 << F_SL2E)
#define M_COLOR_EN  ((MUINT64)1 << F_COLOR)
#define M_MIXER3_EN ((MUINT64)1 << F_MIXER3)

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
    }
    virtual ~IspTuningCustom() {}

    static IspTuningCustom* createInstance(MUINT32 const eSensorDev, MUINT32 const u4SensorID);
    virtual void destroyInstance() = 0;

private:
    MBOOL m_idx_valid;
    MUINT32 m_CCM_DualPara[3];
    MBOOL m_CCM_DualPara_valid;
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
        if (value < low_bound)
        {
            return (low_bound);
        }
        else if (value > upper_bound)
		{
            return (upper_bound);
		}
		else
		{
		    return (value);
		}
    }

    virtual
    MVOID
    RGB2WMatrix(
        const MUINT32 L[], const AWB_GAIN_T& PGN, ISP_NVRAM_CCM_T& W
    );

    virtual
    MVOID
    RGB2YLinear(
        const ISP_NVRAM_CCM_T& inW, ISP_NVRAM_CCM_T& outW
    );

    virtual
    MVOID
    refine_SL2F(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_SL2_T& rSL2F
    );

    virtual
    MVOID
    refine_DBS(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_DBS_T& rDBS
    );

    virtual
    MVOID
    refine_OBC(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_OBC_T& rOBC
    );

    virtual
    MVOID
    refine_BPC(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_BPC_T& rBPC
    );

    virtual
    MVOID
    refine_NR1(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_NR1_T& rNR1
    );

    virtual
    MVOID
    refine_PDC(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_PDC_T& rPDC
    );

    virtual
    MVOID
    refine_SL2(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_SL2_T& rSL2
    );

    virtual
    MVOID
    refine_RPG(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_RPG_T& rPGN
    );

    virtual
    MVOID
    refine_PGN(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_PGN_T& rPGN
    );

    virtual
    MVOID
    refine_RMM(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_RMM_T& rRMM
    );

    virtual
    MVOID
    refine_BOK(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BOK_T& rBOK
    );

    virtual
    MVOID
    refine_RNR(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_RNR_T& rRNR
    );

    virtual
    MVOID
    refine_UDM(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_UDM_T& rUDM
    );

    virtual
    MVOID
    refine_CCM(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCM_T& rCCM
    );

    virtual
    MVOID
    refine_CCM2(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCM_T& rCCM2
    );

    virtual
    MVOID
    refine_GGM(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_GGM_T& rGGM
    );


    virtual
    MVOID
    refine_ANR(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_ANR_T& rANR
    );


    virtual
    MVOID
    refine_ANR2(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_ANR2_T& rANR
    );

    virtual
    MVOID
    refine_CCR(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCR_T& rCCR
    );

    virtual
    MVOID
    refine_ABF(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_ABF_T& rABF
    );

    virtual
    MVOID
    refine_EE(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_EE_T& rEE
    );

    virtual
    MVOID
    refine_HFG(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_HFG_T& rHFG
    );

    virtual
    MVOID
    refine_NR3D(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_NR3D_T& rNR3D
    );

    virtual
    MVOID
    refine_MFB(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_MFB_T& rMFB
    );

    virtual
    MVOID
    refine_MIXER3(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_MIXER3_T& rMIXER3
    );

    virtual
    MVOID
    refine_LCE(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_LCE_T& rLCE
    );

    virtual
    EIndex_CCM_T
    evaluate_CCM_index (
        RAWIspCamInfo const& rCamInfo, EIndex_CCM_T const ePrvIdx
    );

    virtual
    MBOOL
    is_to_invoke_capture_in_color(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MBOOL
    is_to_invoke_isp_interpolation(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MBOOL
    is_to_invoke_flc(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MBOOL
    is_HDRTuning(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MBOOL
    is_ZHDRTuning(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MVOID
    userSetting_EE(
        RAWIspCamInfo const& rCamInfo, EIndex_Isp_Edge_T eIdx_Edge, ISP_NVRAM_EE_T& rEE
    );

    virtual
    MVOID
    userSetting_EFFECT(
        RAWIspCamInfo const& rCamInfo,
        EIndex_Effect_T const& eIdx_Effect,
        IspUsrSelectLevel_T const& rIspUsrSelectLevel,
        ISP_NVRAM_G2C_T& rG2C,
        ISP_NVRAM_G2C_SHADE_T& rG2C_SHADE,
        ISP_NVRAM_SE_T& rSE,
        ISP_NVRAM_GGM_T& rGGM
    );

    virtual
    MVOID
    userSetting_EFFECT_GGM(
        RAWIspCamInfo const& rCamInfo, EIndex_Effect_T const& eIdx_Effect,  ISP_NVRAM_GGM_T& rGGM
    );  // for Lomo GGM

    virtual
    MVOID
    userSetting_EFFECT_GGM_JNI(
        RAWIspCamInfo const& rCamInfo, EIndex_Effect_T const& eIdx_Effect,  ISP_NVRAM_GGM_T& rGGM
    );  // for Lomo JNI GGM

    virtual
	MUINT32
	remap_ISO_value(
	    MUINT32 const u4Iso
    ) const;

    virtual
    MVOID*
    get_feature_control(
        ESensorDev_T eSensorDev
    ) const;

    virtual
    MVOID*
    get_RawHDR_LCEGain_ConverParam(
    ) const;

    virtual
    MBOOL
    is_to_invoke_fine_tune(
        ESensorDev_T const eSensorDev
    );


    virtual
    MVOID
    fine_tune_ANR(
        RAWIspCamInfo const& rCamInfo,
        NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo const& GmaExifInfo,
        NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo const& LceExifInfo,
        ISP_NVRAM_ANR_T const& rANR_U, ISP_NVRAM_ANR_T const& rANR_L,
        ISP_NVRAM_ANR_T& rANR
    );

    virtual
    MVOID
    fine_tune_ANR2(
        RAWIspCamInfo const& rCamInfo,
        NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo const& GmaExifInfo,
        NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo const& LceExifInfo,
        ISP_NVRAM_ANR2_T const& rANR2_U, ISP_NVRAM_ANR2_T const& rANR2_L,
        ISP_NVRAM_ANR2_T& rANR2
    );

    virtual
    MVOID
    fine_tune_EE(
        RAWIspCamInfo const& rCamInfo,
        NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo const& GmaExifInfo,
        NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo const& LceExifInfo,
        ISP_NVRAM_EE_T const& rEE_U, ISP_NVRAM_EE_T const& rEE_L,
        ISP_NVRAM_EE_T& rEE
    );

    virtual
    MVOID
    adaptive_NR3D_setting(RAWIspCamInfo const& rCamInfo,
    ISP_NR3D_SMOOTH_INFO_T const& rNR3D_SW_Params,
    ISP_NVRAM_NR3D_T& rNR3D,
    MINT32& GMVLevelTh,
    ESensorDev_T eSensorDev);

    virtual
    MBOOL
    is_to_invoke_remap_ISP_ISO_with_LV(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MUINT32
    remap_ISP_ISO_with_LV(
        ISP_NVRAM_ISO_INTERVAL_STRUCT const& rISO_ENV, MUINT32 const LV, MUINT32 const realISO, MBOOL enable
    );

    virtual
    MUINT32
    get_HdrDbsNewISO(
    RAWIspCamInfo const& rCamInfo
    );

    virtual
    EIndex_Shading_CCT_T
    evaluate_Shading_CCT_index  (
            RAWIspCamInfo const& rCamInfo
    ) const;

    virtual
    MINT32
    evaluate_Shading_Ratio  (
            RAWIspCamInfo const& rCamInfo,
            MBOOL isSuperNightMode
    );

    virtual
    MVOID
    reset_ISO_SmoothBuffer();

    virtual
    MINT32
    map_AppName_to_MappingInfo(
        const char* pAppname
    );

    virtual
    MINT32
    map_FlashId_to_MappingInfo(
        int flashId
    );

    virtual
    MINT32
    map_P2inSize_to_MappingInfo(
    RAWIspCamInfo const& rCamInfo
    );

    int total_RA_num_frames_;// = 0;        // totol number of frames
    int ISO_Buffer_[10];
    MUINT dynamic_Rto = -1;
};


};  //  NSIspTuning
#endif //  _ISP_TUNING_CUSTOM_H_

