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
#define LOG_TAG "isp_af_config"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>       /* DataType, Ex : MINT32 */
#include <mtkcam/utils/std/Log.h>
#include <af_config.h>

#include <drv/isp_reg.h>

#include <isp_tuning.h>      /* Enum, Ex : ESensorDev_T */

//using namespace NS3Av3;
using namespace NSIspTuning; /* Ex: ESensorDev_Main */

#include <af_cxu_flow.h>
#ifndef REGTABLE_VALUE
#define REGTABLE_VALUE(REG) (pAFRegInfo[ERegInfo_##REG].val) // get value from regtable(reg)
#endif

/* AFConfig end */

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_AF_CONFIG_T&
ISP_AF_CONFIG_T::
getInstance(MINT32 const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  ISP_AF_CONFIG_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_AF_CONFIG_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_MainThird: //  Main Third Sensor
        return  ISP_AF_CONFIG_DEV<ESensorDev_MainThird>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_AF_CONFIG_DEV<ESensorDev_Sub>::getInstance();
    case ESensorDev_SubSecond: //  Sub Second Sensor
        return  ISP_AF_CONFIG_DEV<ESensorDev_SubSecond>::getInstance();
    default:
        CAM_LOGE("eSensorDev = %d", eSensorDev);
        return  ISP_AF_CONFIG_DEV<ESensorDev_Main>::getInstance();
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID ISP_AF_CONFIG_T::resetParam()
{
    m_bIsAFSupport = 0;
    m_u4ConfigNum  = 0;
    memset(m_rAFRegInfoPre, 0, EAFRegInfo_NUM * sizeof(MUINT32));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID ISP_AF_CONFIG_T::AFConfig(AF_CONFIG_T *a_sAFConfig, AF_CONFIG_INFO_T *a_sOutAFInfo, AFRESULT_ISPREG_T *p_sAFResultConfig)
{
    MINT32 hwErr = CxUFlow_AFConfig(a_sAFConfig, p_sAFResultConfig);
    // OutAFInfo for flow control (these could be change if hw constrain happen)
    a_sOutAFInfo->hwConstrainErr = hwErr; // if > 0, skip algo.
    if (a_sOutAFInfo->hwConstrainErr)
    {
        CAM_LOGE("%s HwConstraint 0x%x ==> Keep SkipAlgo", __FUNCTION__, a_sOutAFInfo->hwConstrainErr);
    }
    //=== these could be change if hwconstraint happened ===//
    a_sOutAFInfo->hwBlkNumX      = a_sAFConfig->sConfigReg.AF_BLK_XNUM;
    a_sOutAFInfo->hwBlkNumY      = a_sAFConfig->sConfigReg.AF_BLK_YNUM;
    a_sOutAFInfo->hwBlkSizeX     = a_sAFConfig->sConfigReg.AF_BLK_XSIZE;
    a_sOutAFInfo->hwBlkSizeY     = a_sAFConfig->sConfigReg.AF_BLK_XSIZE;
    a_sOutAFInfo->hwArea.i4X     = a_sAFConfig->sRoi.i4X;
    a_sOutAFInfo->hwArea.i4Y     = a_sAFConfig->sRoi.i4Y;
    a_sOutAFInfo->hwArea.i4W     = a_sAFConfig->sRoi.i4W;
    a_sOutAFInfo->hwArea.i4H     = a_sAFConfig->sRoi.i4H;
    a_sOutAFInfo->hwArea.i4Info  = 0;
    //=== these could be change if hwconstraint happened ===//
    a_sOutAFInfo->hwEnExtMode    = 1;//oblesete

    // AFResultConfig : ResiterTable
    m_u4ConfigNum = a_sAFConfig->u4ConfigNum;

    // Log
    AFRegInfo_T* pAFRegInfo = &p_sAFResultConfig->rAFRegInfo[0];
    CAM_LOGD("HW-%s Config(%d)", __FUNCTION__, p_sAFResultConfig->configNum);
    CAM_LOGD("HW : AFO_XSIZE(0x%x) AFO_YSIZE(0x%x) AF_SIZE(0x%x)",
             REGTABLE_VALUE(AFO_R1_AFO_XSIZE), REGTABLE_VALUE(AFO_R1_AFO_YSIZE), REGTABLE_VALUE(AF_R1_AF_SIZE));
    CAM_LOGD("HW : AF_CON(0x%x) AF_VLD(0x%x) AF_BLK_0(SIZE)(0x%x) AF_BLK_1(NUM)(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_CON), REGTABLE_VALUE(AF_R1_AF_VLD), REGTABLE_VALUE(AF_R1_AF_BLK_0), REGTABLE_VALUE(AF_R1_AF_BLK_1));
    CAM_LOGD("HW : AF_TH_0(0x%x) AF_TH_1(0x%x) AF_TH_2(0x%x) AF_TH_3(0x%x) AF_TH_4(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_TH_0), REGTABLE_VALUE(AF_R1_AF_TH_1), REGTABLE_VALUE(AF_R1_AF_TH_2), REGTABLE_VALUE(AF_R1_AF_TH_3), REGTABLE_VALUE(AF_R1_AF_TH_4));
    CAM_LOGD("HW : AF_SGG1_0(0x%x) AF_SGG1_1(0x%x) AF_SGG1_2(0x%x) AF_SGG1_3(0x%x) AF_SGG1_4(0x%x) AF_SGG1_5(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_SGG1_0), REGTABLE_VALUE(AF_R1_AF_SGG1_1), REGTABLE_VALUE(AF_R1_AF_SGG1_2), REGTABLE_VALUE(AF_R1_AF_SGG1_3), REGTABLE_VALUE(AF_R1_AF_SGG1_4), REGTABLE_VALUE(AF_R1_AF_SGG1_5));
    CAM_LOGD("HW : AF_SGG5_0(0x%x) AF_SGG5_1(0x%x) AF_SGG5_2(0x%x) AF_SGG5_3(0x%x) AF_SGG5_4(0x%x) AF_SGG5_5(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_SGG5_0), REGTABLE_VALUE(AF_R1_AF_SGG5_1), REGTABLE_VALUE(AF_R1_AF_SGG5_2), REGTABLE_VALUE(AF_R1_AF_SGG5_3), REGTABLE_VALUE(AF_R1_AF_SGG5_4), REGTABLE_VALUE(AF_R1_AF_SGG5_5));
    CAM_LOGD("HW : AF_HFLT0_1(0x%x) AF_HFLT0_2(0x%x) AF_HFLT0_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_HFLT0_1), REGTABLE_VALUE(AF_R1_AF_HFLT0_2), REGTABLE_VALUE(AF_R1_AF_HFLT0_3));
    CAM_LOGD("HW : AF_HFLT1_1(0x%x) AF_HFLT1_2(0x%x) AF_HFLT1_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_HFLT1_1), REGTABLE_VALUE(AF_R1_AF_HFLT1_2), REGTABLE_VALUE(AF_R1_AF_HFLT1_3));
    CAM_LOGD("HW : AF_HFLT2_1(0x%x) AF_HFLT2_2(0x%x) AF_HFLT2_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_HFLT2_1), REGTABLE_VALUE(AF_R1_AF_HFLT2_2), REGTABLE_VALUE(AF_R1_AF_HFLT2_3));

    // PL related registers
    CAM_LOGD("HW : AF_R1_AF_CON2(0x%x)", REGTABLE_VALUE(AF_R1_AF_CON2));
    CAM_LOGD("HW : AF_PL_HFLT_1(0x%x) AF_PL_HFLT_2(0x%x) AF_PL_HFLT_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_PL_HFLT_1), REGTABLE_VALUE(AF_R1_AF_PL_HFLT_2), REGTABLE_VALUE(AF_R1_AF_PL_HFLT_3));
    CAM_LOGD("HW : AF_PL_VFLT_1(0x%x) AF_PL_VFLT_2(0x%x) AF_PL_VFLT_3(0x%x)",
             REGTABLE_VALUE(AF_R1_AF_PL_VFLT_1), REGTABLE_VALUE(AF_R1_AF_PL_VFLT_2), REGTABLE_VALUE(AF_R1_AF_PL_VFLT_3));
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL ISP_AF_CONFIG_T::isHWRdy(MUINT32 i4Config)
{
    MBOOL ret = MFALSE;

    if (i4Config >= m_u4ConfigNum)
    {
        if (i4Config == m_u4ConfigNum)
        {
            CAM_LOGD("HW-%s ready(%d)", __FUNCTION__, m_u4ConfigNum);
        }

        ret = MTRUE;
    }

    return ret;
}
