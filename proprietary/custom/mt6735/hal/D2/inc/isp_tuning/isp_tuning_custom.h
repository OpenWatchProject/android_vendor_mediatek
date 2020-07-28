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

namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
class IspTuningCustom
{
protected:  ////    Ctor/Dtor.
    IspTuningCustom() {}
    virtual ~IspTuningCustom() {}

public:
    static IspTuningCustom* createInstance(ESensorDev_T const eSensorDev, MUINT32 const u4SensorID);
    virtual void destroyInstance() = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual ESensorDev_T   getSensorDev() const = 0;
    virtual MUINT32   getSensorID() const = 0;
    virtual INDEX_T const*  getDefaultIndex(EIspProfile_T const eIspProfile,
                                            ESensorMode_T const eSensorMode,
                                            EIndex_Scene_T const eIdx_Scene,
                                            EIndex_ISO_T const eIdx_ISO) const = 0;
    virtual MVOID evaluate_nvram_index(RAWIspCamInfo const& rCamInfo, IndexMgr& rIdxMgr);
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
    refine_OBC(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_OBC_T& rOBC
    );

    virtual
    MVOID
    refine_BPC(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BPC_T& rBPC
    );

    virtual
    MVOID
    refine_NR1(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_NR1_T& rNR1
    );

    virtual
    MVOID*
    get_custom_GMA_env_info(
        ESensorDev_T eSensorDev
    );

    virtual
    MVOID
    refine_SL2(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_SL2_T& rSL2
    );

    virtual
    MVOID
    refine_PGN(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_PGN_T& rPGN
    );

    virtual
    MVOID
    refine_CFA(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CFA_T& rCFA
    );

    virtual
    MVOID
    refine_CCM(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCM_T& rCCM
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
    refine_CCR(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCR_T& rCCR
    );

    virtual
    MVOID
    refine_EE(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_EE_T& rEE
    );

    virtual
    EIndex_CCM_T
    evaluate_CCM_index (
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MBOOL
    is_to_invoke_smooth_ccm_with_preference_gain(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MBOOL
    is_to_invoke_isp_interpolation(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MINT32
    get_CCM_smooth_method(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    EIndex_PCA_LUT_T
    evaluate_PCA_LUT_index  (
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
        //ISP_NVRAM_G2C_SHADE_T& rG2C_SHADE,
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
    EIndex_ISO_T
    map_ISO_value_to_index(
        MUINT32 const u4Iso
    ) const;

    virtual
    MVOID
    refineLightSourceAWBGainforMultiCCM(
        AWB_GAIN_T& rD65, AWB_GAIN_T& rTL84, AWB_GAIN_T& rCWF, AWB_GAIN_T& rA
    );

    virtual
    MUINT32
    map_ISO_index_to_value(
        EIndex_ISO_T const u4IsoIdx
    ) const;

    virtual
    MUINT32
    remap_ISO_value(
    MUINT32 const u4Iso
    ) const;

    virtual
    EIndex_ISO_T
    map_ISO_value_to_upper_index(
        MUINT32 const u4Iso
    ) const;

    virtual
    EIndex_ISO_T
    map_ISO_value_to_lower_index(
        MUINT32 const u4Iso
    ) const;

    virtual
    EIndex_Shading_CCT_T
    evaluate_Shading_CCT_index  (
            RAWIspCamInfo const& rCamInfo
    ) const;

    virtual
    MINT32
    evaluate_Shading_Ratio  (
            RAWIspCamInfo const& rCamInfo
    ) ;

    virtual
    MVOID
    reset_ISO_SmoothBuffer();

    int total_RA_num_frames_;// = 0;        // totol number of frames
    int ISO_Buffer_[10];
};


};  //  NSIspTuning
#endif //  _ISP_TUNING_CUSTOM_H_

