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
#define LOG_TAG "isp_mgr_af_stat"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <drv/tuning_mgr.h>
#include <isp_mgr.h>
#include <isp_mgr_config.h>

#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuning;

#define MY_INST NS3Av3::INST_T<ISP_MGR_AF_STAT_CONFIG_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AF_STAT_CONFIG_T&
ISP_MGR_AF_STAT_CONFIG_T::
getInstance(MUINT32 const eSensorDev)
{
    ISP_MGR_MODULE_GET_INSTANCE(AF_STAT_CONFIG);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
start(MINT32 i4SensorDev, MINT32 i4SensorIdx)
{
    memset(m_rIspRegInfo[EAF_R1],    0, EAFRegInfo_NUM*sizeof(RegInfo_T));
    memset(m_rIspRegInfoPre[EAF_R1], 0, EAFRegInfo_NUM*sizeof(RegInfo_T));

    m_u4ConfigNum   = 0;
    m_bIsApplied    = MFALSE;
    m_bIsAFSupport  = MFALSE;

    m_u4DebugEnable = property_get_int32("vendor.debug.isp_mgr_af.enable", 0);

    CAM_LOGD("[%s] SensorDev(%d) SensorIdx(%d) -", __FUNCTION__, i4SensorDev, i4SensorIdx);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
configReg(AFRESULT_ISPREG_T *pResultConfig)
{
    std::lock_guard<std::mutex> lock(m_Lock);
    for (MUINT32 i = 0; i < EAFRegInfo_NUM; i++)
    {
        //m_rIspRegInfo[EAF_R1][i].addr = pResultConfig->rAFRegInfo[i].addr;
        m_rIspRegInfo[EAF_R1][i].val = pResultConfig->rAFRegInfo[i].val;
    }
    m_u4ConfigNum  = pResultConfig->configNum;
    m_bIsAFSupport = pResultConfig->enableAFHw;
    m_bIsApplied   = pResultConfig->isApplied;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
configUpdate()
{
    if (memcmp( m_rIspRegInfo[EAF_R1], m_rIspRegInfoPre[EAF_R1], EAFRegInfo_NUM*sizeof(RegInfo_T)) != 0)
    {
        memcpy( m_rIspRegInfoPre[EAF_R1], m_rIspRegInfo[EAF_R1], EAFRegInfo_NUM*sizeof(RegInfo_T));

        CAM_LOGD("AFO_XSIZE(0x%x), AFO_YSIZE(0x%x), AF_CON(0x%x), AF_CON2(0x%x), AF_SIZE(0x%x), AF_VLD(0x%x), AF_BLK_PROT(0x%x), AF_BLK_0(0x%x), AF_BLK_1(0x%x)",
                 REG_AF_R1_INFO_VALUE(AFO_R1_AFO_XSIZE), REG_AF_R1_INFO_VALUE(AFO_R1_AFO_YSIZE),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_CON), REG_AF_R1_INFO_VALUE(AF_R1_AF_CON2), REG_AF_R1_INFO_VALUE(AF_R1_AF_SIZE), REG_AF_R1_INFO_VALUE(AF_R1_AF_VLD),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_BLK_PROT), REG_AF_R1_INFO_VALUE(AF_R1_AF_BLK_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_BLK_1));
        CAM_LOGD("AF_HFLT0_1(0x%x), AF_HFLT0_2(0x%x), AF_HFLT0_3(0x%x), AF_HFLT1_1(0x%x), AF_HFLT1_2(0x%x), AF_HFLT1_3(0x%x)",
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT0_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT0_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT0_3),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT1_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT1_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT1_3));
        CAM_LOGD("AF_HFLT2_1(0x%x), AF_HFLT2_2(0x%x), AF_HFLT2_3(0x%x), AF_VFLT_1(0x%x), AF_VFLT_2(0x%x), AF_VFLT_3(0x%x)",
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT2_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT2_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_HFLT2_3),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_VFLT_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_VFLT_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_VFLT_3));
        CAM_LOGD("AF_PL_HFLT_1(0x%x), AF_PL_HFLT_2(0x%x), AF_PL_HFLT_3(0x%x), AF_PL_VFLT_1(0x%x), AF_PL_VFLT_2(0x%x), AF_PL_VFLT_3(0x%x)",
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_PL_HFLT_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_PL_HFLT_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_PL_HFLT_3),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_PL_VFLT_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_PL_VFLT_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_PL_VFLT_3));
        CAM_LOGD("AF_TH_0(0x%x), AF_TH_1(0x%x), AF_TH_2(0x%x), AF_TH_3(0x%x), AF_TH_4(0x%x)",
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_TH_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_TH_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_TH_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_TH_3), REG_AF_R1_INFO_VALUE(AF_R1_AF_TH_4));
        CAM_LOGD("AF_LUT_H0_0(0x%x), AF_LUT_H0_1(0x%x), AF_LUT_H0_2(0x%x), AF_LUT_H0_3(0x%x), AF_LUT_H0_4(0x%x), AF_LUT_H1_0(0x%x), AF_LUT_H1_1(0x%x), AF_LUT_H1_2(0x%x), AF_LUT_H1_3(0x%x), AF_LUT_H1_4(0x%x)",
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H0_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H0_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H0_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H0_3), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H0_4),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H1_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H1_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H1_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H1_3), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H1_4));
        CAM_LOGD("AF_LUT_H2_0(0x%x), AF_LUT_H2_1(0x%x), AF_LUT_H2_2(0x%x), AF_LUT_H2_3(0x%x), AF_LUT_H2_4(0x%x), AF_LUT_V_0(0x%x), AF_LUT_V_1(0x%x), AF_LUT_V_2(0x%x), AF_LUT_V_3(0x%x), AF_LUT_V_4(0x%x)",
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H2_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H2_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H2_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H2_3), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_H2_4),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_V_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_V_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_V_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_V_3), REG_AF_R1_INFO_VALUE(AF_R1_AF_LUT_V_4));
        CAM_LOGD("AF_SGG1_0(0x%x), AF_SGG1_1(0x%x), AF_SGG1_2(0x%x), AF_SGG1_3(0x%x), AF_SGG1_4(0x%x), AF_SGG1_5(0x%x), AF_SGG5_0(0x%x), AF_SGG5_1(0x%x), AF_SGG5_2(0x%x), AF_SGG5_3(0x%x), AF_SGG5_4(0x%x), AF_SGG5_5(0x%x)",
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG1_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG1_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG1_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG1_3), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG1_4), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG1_5),
                 REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG5_0), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG5_1), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG5_2), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG5_3), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG5_4), REG_AF_R1_INFO_VALUE(AF_R1_AF_SGG5_5));
        CAM_LOGD("HW-%s Config(%d)", __FUNCTION__, m_u4ConfigNum);
    }
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
apply(TuningMgr& rTuning, MINT32 &i4Magic, MINT32 i4SubsampleIdex)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_bIsAFSupport != MFALSE)
    {
        if (m_bIsApplied == MTRUE)
        {
            CAM_LOGD("HW-%s Magic(%d) , Config(%d)", __FUNCTION__, i4Magic, m_u4ConfigNum);
            configUpdate();
        }

        rTuning.updateEngine(eTuningMgrFunc_AF_R1,   MTRUE, i4SubsampleIdex);

        AAA_TRACE_DRV(DRV_AF);
        // Register setting
        rTuning.tuningMgrWriteRegs( (TUNING_MGR_REG_IO_STRUCT*)(&(m_rIspRegInfo[EAF_R1][0])),
                                    m_u4RegInfoNum, i4SubsampleIdex);
        AAA_TRACE_END_DRV;

    }
    else
    {
        rTuning.enableEngine(eTuningMgrFunc_AF_R1,   MFALSE, i4SubsampleIdex);
        rTuning.tuningEngine(eTuningMgrFunc_AF_R1,   MFALSE, i4SubsampleIdex);
    }

    dumpRegInfoP1("af_stat_cfg");

    CAM_LOGD_IF(m_u4DebugEnable, "%s", __FUNCTION__);
    return MTRUE;
}


}
