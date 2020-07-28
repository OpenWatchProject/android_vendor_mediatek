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
#define LOG_TAG "isp_mgr_ccm"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "isp_mgr.h"
#include <ccm_mgr.h>

namespace NSIspTuning
{


/*
Q1.N.M, 2's complement, 0 padding at the head no matter positive/negative
no error protection
*/
float cvt2sCompHwFmt2Float(MUINT32 hwfmt, MUINT32 N, MUINT32 M)
{
    bool sig = (hwfmt & (1<<(N+M))); //0: positive; 1: negative
    float ret;
    if (!sig) //positive
    {
        ret = ((float)hwfmt)/(1<<M);
    }
    else
    {
        // 2's complement: NOT -> +1 -> =
        MUINT32 tmp = 0;
        for (int i=0; i<(M+N+1); i++)
        {
            tmp = tmp | (1<<i);
        } //tmp = 111111... with length = 1+N+M

        hwfmt = hwfmt & tmp; //mask for hwfmt to ensure 0 padding at the head
        hwfmt = tmp - hwfmt; //Not operation
        hwfmt++;
        hwfmt = hwfmt & tmp; //ensure no overflow bit
        //now it's positive with the same abs value
        ret = ((float)hwfmt)/(1<<M);
        ret = -ret;
    }
    return ret;
}



inline MVOID fillFloatIntoMUINT32Addr(float val, MUINT32* pDst)
{
    float* ptr = (float*)pDst;
    *ptr = val;
}


MVOID multiply_Y2R_OnTheRight(float* pMat)
{
/*
1.0,  0,            1.40234375
1.0,  -0.349609375, -0.71484375
1.0,  1.771484375,  0
*/
    float y2r[] = {1.0, 0, 1.40234375, 1.0, -0.349609375, -0.71484375, 1.0, 1.771484375, 0};
    float res[9] = {0};

    //Matrix equation: res = pMat * y2r
    
    res[0] = pMat[0]*y2r[0] + pMat[1]*y2r[3] + pMat[2]*y2r[6]; // res11
    res[1] = pMat[0]*y2r[1] + pMat[1]*y2r[4] + pMat[2]*y2r[7]; // res12
    res[2] = pMat[0]*y2r[2] + pMat[1]*y2r[5] + pMat[2]*y2r[8]; // res13
    res[3] = pMat[3]*y2r[0] + pMat[4]*y2r[3] + pMat[5]*y2r[6]; // res21
    res[4] = pMat[3]*y2r[1] + pMat[4]*y2r[4] + pMat[5]*y2r[7]; // res22
    res[5] = pMat[3]*y2r[2] + pMat[4]*y2r[5] + pMat[5]*y2r[8]; // res23
    res[6] = pMat[6]*y2r[0] + pMat[7]*y2r[3] + pMat[8]*y2r[6]; // res31
    res[7] = pMat[6]*y2r[1] + pMat[7]*y2r[4] + pMat[8]*y2r[7]; // res32
    res[8] = pMat[6]*y2r[2] + pMat[7]*y2r[5] + pMat[8]*y2r[8]; // res33

    for (int i=0; i<9; i++) pMat[i] = res[i];

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_CCM_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_CCM_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(ISP_NVRAM_CCM_T const& rParam)
{
    PUT_REG_INFO(CAM_G2G_CNV_1, cnv_1);
    PUT_REG_INFO(CAM_G2G_CNV_2, cnv_2);
    PUT_REG_INFO(CAM_G2G_CNV_3, cnv_3);
    PUT_REG_INFO(CAM_G2G_CNV_4, cnv_4);
    PUT_REG_INFO(CAM_G2G_CNV_5, cnv_5);
    PUT_REG_INFO(CAM_G2G_CNV_6, cnv_6);

    return  (*this);
}

MVOID
ISP_MGR_CCM_T::
putMdpCCM(ISP_NVRAM_CCM_T const& rParam)
{
    convertCcmFormat(rParam, &m_MdpCCM[0]);
}

MVOID
ISP_MGR_CCM_T::
getMdpCCM(ISP_NVRAM_CCM_T& rParam)
{
    convertCcmFormat(&m_MdpCCM[0], rParam);
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_T& rParam)
{
    GET_REG_INFO(CAM_G2G_CNV_1, cnv_1);
    GET_REG_INFO(CAM_G2G_CNV_2, cnv_2);
    GET_REG_INFO(CAM_G2G_CNV_3, cnv_3);
    GET_REG_INFO(CAM_G2G_CNV_4, cnv_4);
    GET_REG_INFO(CAM_G2G_CNV_5, cnv_5);
    GET_REG_INFO(CAM_G2G_CNV_6, cnv_6);

    return  (*this);
}

MBOOL
ISP_MGR_CCM_T::
apply(EIspProfile_T eIspProfile)
{
    addressErrorCheck("Before ISP_MGR_CCM_T::apply()");

    if (!isEnable()) { // Reset to unit matrix
        reinterpret_cast<ISP_CAM_G2G_CNV_1_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_1))->G2G_CNV_00 = 512;
        reinterpret_cast<ISP_CAM_G2G_CNV_1_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_1))->G2G_CNV_01 = 0;
        reinterpret_cast<ISP_CAM_G2G_CNV_2_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_2))->G2G_CNV_02 = 0;
        reinterpret_cast<ISP_CAM_G2G_CNV_3_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_3))->G2G_CNV_10 = 0;
        reinterpret_cast<ISP_CAM_G2G_CNV_3_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_3))->G2G_CNV_11 = 512;
        reinterpret_cast<ISP_CAM_G2G_CNV_4_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_4))->G2G_CNV_12 = 0;
        reinterpret_cast<ISP_CAM_G2G_CNV_5_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_5))->G2G_CNV_20 = 0;
        reinterpret_cast<ISP_CAM_G2G_CNV_5_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_5))->G2G_CNV_21 = 0;
        reinterpret_cast<ISP_CAM_G2G_CNV_6_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CNV_6))->G2G_CNV_22 = 512;
    }

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_G2G(isEnable());

    TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                           eTuningMgrFunc_G2g);

    // TOP 
    TUNING_MGR_WRITE_ENABLE_BITS(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), CAM_CTL_EN_P2, G2G_EN, isEnable());

    // CAM_G2G_CTRL
    reinterpret_cast<ISP_CAM_G2G_CTRL_T*>(REG_INFO_VALUE_PTR(CAM_G2G_CTRL))->G2G_ACC = 9; // Q1.3.9
    
    TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                                 static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), 
                                                 ERegInfo_CAM_G2G_CTRL+1);
    if(1){//set ISO to MDP
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.isp.iso.ctrl", value, "-1"); // -1: default, 0: unit matrix
        MINT32 ctrl = atoi(value);

        if (ctrl != -1) *reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_ISO_ADAP)) = ctrl;
        else            *reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_ISO_ADAP)) = m_u4ISO;

        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                           eTuningMgrFunc_Iso);

        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                                 reinterpret_cast<TUNING_MGR_REG_IO_STRUCT*>(&m_rIspRegInfo[ERegInfo_CAM_ISO_ADAP]), 
                                                 1);
    }
    if (m_bRWBSensor)
    {

        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("isp.mdp_ccm.ctrl", value, "-1"); // -1: default, 0: unit matrix
        MINT32 ctrl = atoi(value);

        if (ctrl == 0)
        {
            m_MdpCCM_Y2R[0] = 1.0;
            m_MdpCCM_Y2R[1] = 0.0;
            m_MdpCCM_Y2R[2] = 0.0;
            m_MdpCCM_Y2R[3] = 0.0;
            m_MdpCCM_Y2R[4] = 1.0;
            m_MdpCCM_Y2R[5] = 0.0;
            m_MdpCCM_Y2R[6] = 0.0;
            m_MdpCCM_Y2R[7] = 0.0;
            m_MdpCCM_Y2R[8] = 1.0;

            multiply_Y2R_OnTheRight(&m_MdpCCM_Y2R[0]);
#if 0
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[0],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX1)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[1],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX2)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[2],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX3)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[3],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX4)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[4],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX5)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[5],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX6)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[6],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX7)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[7],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX8)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[8],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX9)));
#endif

            
            MY_LOG_IF(1, "set MDP CCM to unit matrix, 1.0(0x%8x), 0.0(0x%8x)"
                , *reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX1))
                , *reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX2)));
        }
        else //ctrl = -1 or others
        {
            m_MdpCCM_Y2R[0] = cvt2sCompHwFmt2Float(m_MdpCCM[0], 3, 9);
            m_MdpCCM_Y2R[1] = cvt2sCompHwFmt2Float(m_MdpCCM[1], 3, 9);
            m_MdpCCM_Y2R[2] = cvt2sCompHwFmt2Float(m_MdpCCM[2], 3, 9);
            m_MdpCCM_Y2R[3] = cvt2sCompHwFmt2Float(m_MdpCCM[3], 3, 9);
            m_MdpCCM_Y2R[4] = cvt2sCompHwFmt2Float(m_MdpCCM[4], 3, 9);
            m_MdpCCM_Y2R[5] = cvt2sCompHwFmt2Float(m_MdpCCM[5], 3, 9);
            m_MdpCCM_Y2R[6] = cvt2sCompHwFmt2Float(m_MdpCCM[6], 3, 9);
            m_MdpCCM_Y2R[7] = cvt2sCompHwFmt2Float(m_MdpCCM[7], 3, 9);
            m_MdpCCM_Y2R[8] = cvt2sCompHwFmt2Float(m_MdpCCM[8], 3, 9);

            multiply_Y2R_OnTheRight(&m_MdpCCM_Y2R[0]);
#if 0
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[0],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX1)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[1],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX2)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[2],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX3)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[3],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX4)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[4],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX5)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[5],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX6)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[6],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX7)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[7],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX8)));
            fillFloatIntoMUINT32Addr(m_MdpCCM_Y2R[8],reinterpret_cast<MUINT32*>(REG_INFO_VALUE_PTR(CAM_RWB_MATRIX9)));
#endif

            /*
            CCM[0] =  M11
            CCM[1] =  M12
            CCM[2] =  M13
            CCM[3] =  M21
            CCM[4] =  M22
            CCM[5] =  M23
            CCM[6] =  M31
            CCM[7] =  M32
            CCM[8] =  M33
            */
        }
        
        TuningMgr::getInstance().updateEngine(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                           eTuningMgrFunc_Rwb);

        TuningMgr::getInstance().tuningMgrWriteRegs(static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]), 
                                                 reinterpret_cast<TUNING_MGR_REG_IO_STRUCT*>(&m_rIspRegInfo[ERegInfo_CAM_RWB_MATRIX1]), 
                                                 9);
    }


    addressErrorCheck("After ISP_MGR_CCM_T::apply()");

    dumpRegInfo("G2G");

    return  MTRUE;
}


}
