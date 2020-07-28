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
#define LOG_TAG "isp_mgr_ae_stat"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include "property_utils.h"
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <drv/isp_reg.h>
#include "isp_mgr.h"
#include "isp_mgr_config.h"

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AE_STAT_CONFIG_T&
ISP_MGR_AE_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(AE_STAT_CONFIG);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AE_STAT_CONFIG_T::
configReg(MVOID *pIspRegInfo)
{
    memcpy(m_rIspRegInfo[EAE_R1], (RegInfo_T *)pIspRegInfo, EAERegInfo_NUM*sizeof(RegInfo_T));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AE RAW Pre-gain2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
setIspAEPreGain2(AWB_GAIN_T& rIspAWBGain)
{
    m_rIspAEPreGain2 = rIspAWBGain;

    if( (m_rIspAEPreGain2.i4G <512)||(m_rIspAEPreGain2.i4R <512)||(m_rIspAEPreGain2.i4B <512)){
        CAM_LOGD("[AEconfig pregain2 Warning] R/G/B = %d/%d/%d \n", m_rIspAEPreGain2.i4R, m_rIspAEPreGain2.i4G, m_rIspAEPreGain2.i4B);
        m_rIspAEPreGain2.i4R = 512;
        m_rIspAEPreGain2.i4G = 512;
        m_rIspAEPreGain2.i4B = 512;
    }
#if 0
    // CAM_AE_GAIN2_0
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_R = m_rIspAEPreGain2.i4R;
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_0_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_0))->RAWPREGAIN2_G = m_rIspAEPreGain2.i4G;
    // CAM_REG_AE_GAIN2_1
    reinterpret_cast<ISP_CAM_AE_RAWPREGAIN2_1_T*>(REG_INFO_VALUE_PTR(CAM_AE_GAIN2_1))->RAWPREGAIN2_B = m_rIspAEPreGain2.i4B;
#endif
    return MTRUE;
}

MBOOL
ISP_MGR_AE_STAT_CONFIG_T::
apply(TuningMgr& rTuning, MINT32 i4SubsampleIdex)
{
    rTuning.updateEngine(eTuningMgrFunc_AA_R1, MTRUE, i4SubsampleIdex);
    //rTuning.updateEngine(eTuningMgrFunc_PS, MTRUE, i4SubsampleIdex);

    // Register setting

    TUNING_MGR_WRITE_BITS_CAM(&rTuning, CAMCTL_R1_CAMCTL_EN,CAMCTL_AA_R1_EN, 1, i4SubsampleIdex);

    AAA_TRACE_DRV(DRV_AE);
    rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[EAE_R1][0])),
        m_u4RegInfoNum, i4SubsampleIdex);
    AAA_TRACE_END_DRV;


    dumpRegInfoP1("ae_stat_cfg");
    return MTRUE;
}

}
