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
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_mgr_helper.h>
#include "ccm_mgr.h"
#include <mtkcam/algorithm/lib3a/isp_algo_common.h>


using namespace NSIspTuning;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class CcmMgrDev : public CcmMgr
{
public:
    static
    CcmMgr*
    getInstance(ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, ISP_NVRAM_MULTI_CCM_STRUCT& rMDPMultiCCM, IspTuningCustom* pIspTuningCustom)
    {
        static CcmMgrDev<eSensorDev> singleton(rIspNvramReg, rISPMultiCCM, rMDPMultiCCM, pIspTuningCustom);
        return &singleton;
    }
    virtual MVOID destroyInstance() {}

    CcmMgrDev(ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, ISP_NVRAM_MULTI_CCM_STRUCT& rMDPMultiCCM, IspTuningCustom* pIspTuningCustom)
        : CcmMgr(eSensorDev, rIspNvramReg, rISPMultiCCM, rMDPMultiCCM, pIspTuningCustom)
    {}

    virtual ~CcmMgrDev() {}

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INSTANTIATE(_dev_id) \
    case _dev_id: return  CcmMgrDev<_dev_id>::getInstance(rIspNvramReg, rISPMultiCCM, rMDPMultiCCM, pIspTuningCustom)

CcmMgr*
CcmMgr::
createInstance(ESensorDev_T const eSensorDev, ISP_NVRAM_REGISTER_STRUCT& rIspNvramReg, ISP_NVRAM_MULTI_CCM_STRUCT& rISPMultiCCM, ISP_NVRAM_MULTI_CCM_STRUCT& rMDPMultiCCM, IspTuningCustom* pIspTuningCustom)
{
    switch  (eSensorDev)
    {
    INSTANTIATE(ESensorDev_Main);       //  Main Sensor
    INSTANTIATE(ESensorDev_MainSecond); //  Main Second Sensor
    INSTANTIATE(ESensorDev_Sub);        //  Sub Sensor
    default:
        break;
    }

    return  MNULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
CcmMgr::
calculateCCM(AWB_INFO_T const& rAWBInfo, MBOOL bWithPreferenceGain, MINT32 i4FlashOnOff, MINT32 i4SmoothMethod, MBOOL bRWBSensor, MINT32 ProMehod, MINT32 ProMax)
{
    {
    if (bWithPreferenceGain)
        SmoothCCM(rAWBInfo.rCurrentAWBGain, 3, 9, m_rISPMultiCCM, m_rCCMOutput[E_ISP_CCM], m_rCCMWeight, i4FlashOnOff, i4SmoothMethod, ProMehod, ProMax);
    else
        SmoothCCM(rAWBInfo.rCurrentFullAWBGain, 3, 9, m_rISPMultiCCM, m_rCCMOutput[E_ISP_CCM], m_rCCMWeight, i4FlashOnOff, i4SmoothMethod, ProMehod, ProMax);
    }

    if (bRWBSensor)
    {
        if (bWithPreferenceGain)
            SmoothCCM(rAWBInfo.rCurrentAWBGain, 3, 9, m_rMDPMultiCCM, m_rCCMOutput[E_MDP_CCM], m_rMdpCCMWeight, i4FlashOnOff, i4SmoothMethod, ProMehod, ProMax);
        else
            SmoothCCM(rAWBInfo.rCurrentFullAWBGain, 3, 9, m_rMDPMultiCCM, m_rCCMOutput[E_MDP_CCM], m_rMdpCCMWeight, i4FlashOnOff, i4SmoothMethod, ProMehod, ProMax);
    }
}

MVOID
CcmMgr::separateCCM4Gpu(MFLOAT isp_ccm_ratio)
{
    //separate ISP_CCM(100%) into ISP_CCM(ratio) and GPU_CCM(1-ratio) by given ratio
    // input: m_rCCMOutput[E_ISP_CCM]
    // output: m_rCCMOutput[E_ISP_CCM] with ratio & m_rCCMOutput[E_GPU_CCM] with 1-ratio

    MINT32 isp_ccm[3*3];
    MINT32 isp_ccm_new[3*3];
    MINT32 gpu_ccm[3*3];
    convertCcmFormat(m_rCCMOutput[E_ISP_CCM], &isp_ccm[0]);

    separate_CCM3x3_by_ratio(&isp_ccm[0], isp_ccm_ratio, 3, 9, &isp_ccm_new[0], &gpu_ccm[0]); //format: Q1.3.9

    convertCcmFormat(&gpu_ccm[0], m_rCCMOutput[E_GPU_CCM]);
    convertCcmFormat(&isp_ccm_new[0], m_rCCMOutput[E_ISP_CCM]);
}

MBOOL 
CcmMgr::
gen_CM_FM(AWB_GAIN_T const& rCurrent, double in_CCM[3][3], double out_CM[3][3], double out_FM[3][3])
{
    int i,j;
    double CA[3][3] =     {{1.0478112,  0.0228866, -0.0501270},{ 0.0295424, 0.9904844, -0.0170491},{-0.0092345,  0.0150436, 0.7521316}};
    double inv_CA[3][3] = {{0.9555766, -0.0230393,  0.0631636},{-0.0282895, 1.0099416,  0.0210077},{ 0.0122982, -0.0204830, 1.3299098}};
    double M[3][3] =      {{3.2404542, -1.5371385, -0.4985314},{-0.9692660, 1.8760108,  0.0415560},{ 0.0556434, -0.2040259, 1.0572252}};
    double inv_M[3][3] =  {{0.4124564,  0.3575761,  0.1804375},{ 0.2126729, 0.7151522,  0.0721750},{ 0.0193339,  0.1191920, 0.9503041}};

		// Merlin 2015-07-17: Generate new diagonal CCM entries by multiplying WB gains
    double wb_CCM[3][3] = {{in_CCM[0][0]*(double)rCurrent.i4R/512.0,in_CCM[0][1]*(double)rCurrent.i4G/512.0,in_CCM[0][2]*(double)rCurrent.i4B/512.0},
    	                     {in_CCM[1][0]*(double)rCurrent.i4R/512.0,in_CCM[1][1]*(double)rCurrent.i4G/512.0,in_CCM[1][2]*(double)rCurrent.i4B/512.0},
                           {in_CCM[2][0]*(double)rCurrent.i4R/512.0,in_CCM[2][1]*(double)rCurrent.i4G/512.0,in_CCM[2][2]*(double)rCurrent.i4B/512.0}};

    double det = wb_CCM[0][0] * (wb_CCM[1][1] * wb_CCM[2][2] - wb_CCM[2][1] * wb_CCM[1][2]) -
                wb_CCM[0][1] * (wb_CCM[1][0] * wb_CCM[2][2] - wb_CCM[1][2] * wb_CCM[2][0]) +
                wb_CCM[0][2] * (wb_CCM[1][0] * wb_CCM[2][1] - wb_CCM[1][1] * wb_CCM[2][0]);

    if (det == 0.0)
    {
        MY_ERR("Fail to inverse the matrix");
        return MFALSE;
    }

    double invdet = 1.0 / det;

    double inv_CCM[3][3];
    
    //inv_CCM[0][0] = (in_CCM[1][1] * in_CCM[2][2] - in_CCM[2][1] * in_CCM[1][2]) * invdet;
    //inv_CCM[0][1] = (in_CCM[0][2] * in_CCM[2][1] - in_CCM[0][1] * in_CCM[2][2]) * invdet;
    //inv_CCM[0][2] = (in_CCM[0][1] * in_CCM[1][2] - in_CCM[0][2] * in_CCM[1][1]) * invdet;
    //inv_CCM[1][0] = (in_CCM[1][2] * in_CCM[2][0] - in_CCM[1][0] * in_CCM[2][2]) * invdet;
    //inv_CCM[1][1] = (in_CCM[0][0] * in_CCM[2][2] - in_CCM[0][2] * in_CCM[2][0]) * invdet;
    //inv_CCM[1][2] = (in_CCM[1][0] * in_CCM[0][2] - in_CCM[0][0] * in_CCM[1][2]) * invdet;
    //inv_CCM[2][0] = (in_CCM[1][0] * in_CCM[2][1] - in_CCM[2][0] * in_CCM[1][1]) * invdet;
    //inv_CCM[2][1] = (in_CCM[2][0] * in_CCM[0][1] - in_CCM[0][0] * in_CCM[2][1]) * invdet;
    //inv_CCM[2][2] = (in_CCM[0][0] * in_CCM[1][1] - in_CCM[1][0] * in_CCM[0][1]) * invdet;
    inv_CCM[0][0] = (wb_CCM[1][1] * wb_CCM[2][2] - wb_CCM[2][1] * wb_CCM[1][2]) * invdet;
    inv_CCM[0][1] = (wb_CCM[0][2] * wb_CCM[2][1] - wb_CCM[0][1] * wb_CCM[2][2]) * invdet;
    inv_CCM[0][2] = (wb_CCM[0][1] * wb_CCM[1][2] - wb_CCM[0][2] * wb_CCM[1][1]) * invdet;
    inv_CCM[1][0] = (wb_CCM[1][2] * wb_CCM[2][0] - wb_CCM[1][0] * wb_CCM[2][2]) * invdet;
    inv_CCM[1][1] = (wb_CCM[0][0] * wb_CCM[2][2] - wb_CCM[0][2] * wb_CCM[2][0]) * invdet;
    inv_CCM[1][2] = (wb_CCM[1][0] * wb_CCM[0][2] - wb_CCM[0][0] * wb_CCM[1][2]) * invdet;
    inv_CCM[2][0] = (wb_CCM[1][0] * wb_CCM[2][1] - wb_CCM[2][0] * wb_CCM[1][1]) * invdet;
    inv_CCM[2][1] = (wb_CCM[2][0] * wb_CCM[0][1] - wb_CCM[0][0] * wb_CCM[2][1]) * invdet;
    inv_CCM[2][2] = (wb_CCM[0][0] * wb_CCM[1][1] - wb_CCM[1][0] * wb_CCM[0][1]) * invdet;

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            out_CM[i][j] = inv_CCM[i][0]*M[0][j] + inv_CCM[i][1]*M[1][j] + inv_CCM[i][2]*M[2][j];
        }
    }

    double CA_inv_M[3][3];

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            CA_inv_M[i][j] = CA[i][0]*inv_M[0][j] + CA[i][1]*inv_M[1][j] + CA[i][2]*inv_M[2][j];
        }
    }

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            out_FM[i][j] = CA_inv_M[i][0]*in_CCM[0][j] + CA_inv_M[i][1]*in_CCM[1][j] + CA_inv_M[i][2]*in_CCM[2][j];
        }
    }

    return MTRUE;
}
