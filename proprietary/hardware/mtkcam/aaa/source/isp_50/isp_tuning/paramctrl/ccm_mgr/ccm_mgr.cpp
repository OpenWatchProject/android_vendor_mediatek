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
#define LOG_TAG "ccm_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "ccm_mgr.h"
#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;
using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<CcmMgr>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuningv3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if MTK_CAM_NEW_NVRAM_SUPPORT
CcmMgr*
CcmMgr::
createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_COLOR_TABLE_STRUCT& rNewColor, IspTuningCustom* pIspTuningCustom)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(static_cast<MINT32>(eSensorDev));

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<CcmMgr>(eSensorDev, rNewColor, pIspTuningCustom);
    } );

    return rSingleton.instance.get();
}

#if 1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CcmMgr::
calculateCCM(ISP_NVRAM_CCM_T& rCCMout, MUINT16& rVer, RAWIspCamInfo const& rIspCamInfo, ISP_NVRAM_MULTI_CCM_STRUCT& rCCMTuningUpper, ISP_NVRAM_MULTI_CCM_STRUCT& rCCMTuningLower, MINT32 i4UpperLv, MINT32 i4LowerLv)
{
ColorInfo rColorInfo;
memset(&rColorInfo, 0, sizeof(ColorInfo));

rColorInfo.RealLV = rIspCamInfo.rAEInfo.i4RealLightValue_x10;
rColorInfo.AWB_NoPrefGain[0] = rIspCamInfo.rAWBInfo.rAwbGainNoPref.i4R;
rColorInfo.AWB_NoPrefGain[1] = rIspCamInfo.rAWBInfo.rAwbGainNoPref.i4G;
rColorInfo.AWB_NoPrefGain[2] = rIspCamInfo.rAWBInfo.rAwbGainNoPref.i4B;
rColorInfo.NoPrefCCT = rIspCamInfo.rAWBInfo.i4CCT;

SmoothCCM( 3, 9, rCCMout, rVer,
                i4UpperLv, // LV ¤W¬É­È
                i4LowerLv, // LV ¤U¬É­È
                rCCMTuningUpper, // LV ¤W¬Éidx
                rCCMTuningLower, // LV ¤U¬É idx
                rColorInfo, // ·s¼W¦Û©w¸qªºstructure
                m_SmoothCCM_Record,
                m_SmoothCCM_Counter);
}

#endif
MVOID
CcmMgr::RWB_CCM_decomposition(ISP_NVRAM_CCM_T& ccm_out, MFLOAT isp_ccm_ratio)
{
    //separate ISP_CCM(100%) into ISP_CCM(ratio) and GPU_CCM(1-ratio) by given ratio
    // input: m_rCCMOutput[E_ISP_CCM]
    // output: m_rCCMOutput[E_ISP_CCM] with ratio & m_rCCMOutput[E_GPU_CCM] with 1-ratio

    //seperate_RWB_CCM_by_ratio(&ccm_out, isp_ccm_ratio, 3, 9);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_NVRAM_CCM_T&
CcmMgr::
getCCM()
{
        return m_rColorCorrectionTransform;
}

ISP_NVRAM_CCM_T&
CcmMgr::
getPrvCCM()
{
    return m_rCCMOutput;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CcmMgr::
setColorCorrectionTransform(ISP_CCM_T& rCCM)
{
    // M11
    if (rCCM.M11 < 0)
        m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_00 = 8192 + rCCM.M11; // 2's complement
    else
        m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_00 = rCCM.M11;

     // M12
     if (rCCM.M12 < 0)
         m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_01 = 8192 + rCCM.M12; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_01 = rCCM.M12;

     // M13
     if (rCCM.M13 < 0)
         m_rColorCorrectionTransform.cnv_2.bits.G2G_CNV_02 = 8192 + rCCM.M13; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_2.bits.G2G_CNV_02 = rCCM.M13;

     // M21
     if (rCCM.M21 < 0)
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_10 = 8192 + rCCM.M21; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_10 = rCCM.M21;

     // M22
     if (rCCM.M22 < 0)
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_11 = 8192 + rCCM.M22; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_11 = rCCM.M22;

     // M23
     if (rCCM.M23 < 0)
         m_rColorCorrectionTransform.cnv_4.bits.G2G_CNV_12 = 8192 + rCCM.M23; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_4.bits.G2G_CNV_12 = rCCM.M23;

     // M31
     if (rCCM.M31 < 0)
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_20 = 8192 + rCCM.M31; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_20 = rCCM.M31;

     // M32
     if (rCCM.M32 < 0)
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_21 = 8192 + rCCM.M32; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_21 = rCCM.M32;

     // M33
     if (rCCM.M33 < 0)
         m_rColorCorrectionTransform.cnv_6.bits.G2G_CNV_22 = 8192 + rCCM.M33; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_6.bits.G2G_CNV_22 = rCCM.M33;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static MINT32 Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result;

    if (((value >> (digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }

    return Result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CcmMgr::
getCCM(ISP_CCM_T& rCCM)
{
    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) {
        rCCM.M11 = Complement2(m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_00,13);
        rCCM.M12 = Complement2(m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_01,13);
        rCCM.M13 = Complement2(m_rColorCorrectionTransform.cnv_2.bits.G2G_CNV_02,13);
        rCCM.M21 = Complement2(m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_10,13);
        rCCM.M22 = Complement2(m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_11,13);
        rCCM.M23 = Complement2(m_rColorCorrectionTransform.cnv_4.bits.G2G_CNV_12,13);
        rCCM.M31 = Complement2(m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_20,13);
        rCCM.M32 = Complement2(m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_21,13);
        rCCM.M33 = Complement2(m_rColorCorrectionTransform.cnv_6.bits.G2G_CNV_22,13);
    }
    else {
        rCCM.M11 = Complement2(m_rCCMOutput.cnv_1.bits.G2G_CNV_00,13);
        rCCM.M12 = Complement2(m_rCCMOutput.cnv_1.bits.G2G_CNV_01,13);
        rCCM.M13 = Complement2(m_rCCMOutput.cnv_2.bits.G2G_CNV_02,13);
        rCCM.M21 = Complement2(m_rCCMOutput.cnv_3.bits.G2G_CNV_10,13);
        rCCM.M22 = Complement2(m_rCCMOutput.cnv_3.bits.G2G_CNV_11,13);
        rCCM.M23 = Complement2(m_rCCMOutput.cnv_4.bits.G2G_CNV_12,13);
        rCCM.M31 = Complement2(m_rCCMOutput.cnv_5.bits.G2G_CNV_20,13);
        rCCM.M32 = Complement2(m_rCCMOutput.cnv_5.bits.G2G_CNV_21,13);
        rCCM.M33 = Complement2(m_rCCMOutput.cnv_6.bits.G2G_CNV_22,13);
    }
}
#else
template <ESensorDev_T const eSensorDev>
class CcmMgrDev : public CcmMgr
{
public:
    static
    CcmMgr*
    getInstance(ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, IspTuningCustom* pIspTuningCustom)
    {
        static CcmMgrDev<eSensorDev> singleton(rISPMultiCCM, pIspTuningCustom);
        return &singleton;
    }
    virtual MVOID destroyInstance() {}
    CcmMgrDev(ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, IspTuningCustom* pIspTuningCustom)
        : CcmMgr(eSensorDev, rISPMultiCCM, pIspTuningCustom)
    {}
    virtual ~CcmMgrDev() {}
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INSTANTIATE(_dev_id) \
    case _dev_id: return  CcmMgrDev<_dev_id>::getInstance(rISPMultiCCM, pIspTuningCustom)
CcmMgr*
CcmMgr::
createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, IspTuningCustom* pIspTuningCustom)
{
    switch  (eSensorDev)
    {
    INSTANTIATE(ESensorDev_Main);       //  Main Sensor
    INSTANTIATE(ESensorDev_MainSecond); //  Main Second Sensor
    INSTANTIATE(ESensorDev_MainThird); //  Main Third Sensor
    INSTANTIATE(ESensorDev_Sub);        //  Sub Sensor
    INSTANTIATE(ESensorDev_SubSecond); //  Main Second Sensor
    default:
        break;
    }
    return  MNULL;
}
#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CcmMgr::
calculateCCM(ISP_NVRAM_CCM_T& rCCM, MUINT16& rVer, RAWIspCamInfo const& rIspCamInfo, ISP_NVRAM_MULTI_CCM_STURCT& rCCMTuning)
{
#if 0
//     SmoothCCM(bWithPreferenceGain, 3, 9, m_rISPMultiCCM, rCCM, rVer, rIspCamInfo,
//              m_SmoothCCM_Record, m_SmoothCCM_Counter, m_SmoothCCM_Coef);
// Assign CamInfo to Algo Struct
MVOID SmoothCCM( 3, 9, rCCMTuning, rCCM, rVer,
                MINT16 UpperLV, // LV ?W?E-E
                MINT16 LowerLV, // LV ?U?E-E
                MUINT16 UpperIdx, // LV ?W?Eidx
                MUINT16 LowerIdx, // LV ?U?E idx
                ColorInfo& rColorInfo, // ¡Ps?W|Ucw¡Mqaostructure
                ISP_NVRAM_CCM_T* pSmoothCCM_Record,
                MUINT32& rSmoothCCM_Counter )
#endif
}
#endif
MVOID
CcmMgr::RWB_CCM_decomposition(ISP_NVRAM_CCM_T& ccm_out, MFLOAT isp_ccm_ratio)
{
    //separate ISP_CCM(100%) into ISP_CCM(ratio) and GPU_CCM(1-ratio) by given ratio
    // input: m_rCCMOutput[E_ISP_CCM]
    // output: m_rCCMOutput[E_ISP_CCM] with ratio & m_rCCMOutput[E_GPU_CCM] with 1-ratio
    //seperate_RWB_CCM_by_ratio(&ccm_out, isp_ccm_ratio, 3, 9);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_NVRAM_CCM_T&
CcmMgr::
getCCM()
{
        return m_rColorCorrectionTransform;
}
ISP_NVRAM_CCM_T&
CcmMgr::
getPrvCCM()
{
    return m_rCCMOutput;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CcmMgr::
setColorCorrectionTransform(ISP_CCM_T& rCCM)
{
    // M11
    if (rCCM.M11 < 0)
        m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_00 = 8192 + rCCM.M11; // 2's complement
    else
        m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_00 = rCCM.M11;
     // M12
     if (rCCM.M12 < 0)
         m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_01 = 8192 + rCCM.M12; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_01 = rCCM.M12;
     // M13
     if (rCCM.M13 < 0)
         m_rColorCorrectionTransform.cnv_2.bits.G2G_CNV_02 = 8192 + rCCM.M13; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_2.bits.G2G_CNV_02 = rCCM.M13;
     // M21
     if (rCCM.M21 < 0)
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_10 = 8192 + rCCM.M21; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_10 = rCCM.M21;
     // M22
     if (rCCM.M22 < 0)
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_11 = 8192 + rCCM.M22; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_11 = rCCM.M22;
     // M23
     if (rCCM.M23 < 0)
         m_rColorCorrectionTransform.cnv_4.bits.G2G_CNV_12 = 8192 + rCCM.M23; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_4.bits.G2G_CNV_12 = rCCM.M23;
     // M31
     if (rCCM.M31 < 0)
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_20 = 8192 + rCCM.M31; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_20 = rCCM.M31;
     // M32
     if (rCCM.M32 < 0)
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_21 = 8192 + rCCM.M32; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_21 = rCCM.M32;
     // M33
     if (rCCM.M33 < 0)
         m_rColorCorrectionTransform.cnv_6.bits.G2G_CNV_22 = 8192 + rCCM.M33; // 2's complement
     else
         m_rColorCorrectionTransform.cnv_6.bits.G2G_CNV_22 = rCCM.M33;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static MINT32 Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result;
    if (((value >> (digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }
    return Result;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CcmMgr::
getCCM(ISP_CCM_T& rCCM)
{
    if (m_eColorCorrectionMode == MTK_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) {
        rCCM.M11 = Complement2(m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_00,13);
        rCCM.M12 = Complement2(m_rColorCorrectionTransform.cnv_1.bits.G2G_CNV_01,13);
        rCCM.M13 = Complement2(m_rColorCorrectionTransform.cnv_2.bits.G2G_CNV_02,13);
        rCCM.M21 = Complement2(m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_10,13);
        rCCM.M22 = Complement2(m_rColorCorrectionTransform.cnv_3.bits.G2G_CNV_11,13);
        rCCM.M23 = Complement2(m_rColorCorrectionTransform.cnv_4.bits.G2G_CNV_12,13);
        rCCM.M31 = Complement2(m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_20,13);
        rCCM.M32 = Complement2(m_rColorCorrectionTransform.cnv_5.bits.G2G_CNV_21,13);
        rCCM.M33 = Complement2(m_rColorCorrectionTransform.cnv_6.bits.G2G_CNV_22,13);
    }
    else {
        rCCM.M11 = Complement2(m_rCCMOutput.cnv_1.bits.G2G_CNV_00,13);
        rCCM.M12 = Complement2(m_rCCMOutput.cnv_1.bits.G2G_CNV_01,13);
        rCCM.M13 = Complement2(m_rCCMOutput.cnv_2.bits.G2G_CNV_02,13);
        rCCM.M21 = Complement2(m_rCCMOutput.cnv_3.bits.G2G_CNV_10,13);
        rCCM.M22 = Complement2(m_rCCMOutput.cnv_3.bits.G2G_CNV_11,13);
        rCCM.M23 = Complement2(m_rCCMOutput.cnv_4.bits.G2G_CNV_12,13);
        rCCM.M31 = Complement2(m_rCCMOutput.cnv_5.bits.G2G_CNV_20,13);
        rCCM.M32 = Complement2(m_rCCMOutput.cnv_5.bits.G2G_CNV_21,13);
        rCCM.M33 = Complement2(m_rCCMOutput.cnv_6.bits.G2G_CNV_22,13);
    }
}
#endif
};
