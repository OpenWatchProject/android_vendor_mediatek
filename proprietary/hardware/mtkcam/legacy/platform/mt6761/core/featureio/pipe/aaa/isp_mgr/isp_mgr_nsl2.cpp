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
#define LOG_TAG "isp_mgr_nsl2"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <mtkcam/drv/isp_drv.h>

#include "isp_mgr.h"
#include <mtkcam/featureio/tuning_mgr.h>

#define CLAMP(x,min,max)       (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define SLP_PREC_F_SCAL 256


namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SL2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_NSL2_T&
ISP_MGR_NSL2_T::
getInstance(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
{
    case ESensorDev_Main: //  Main Sensor
        return  ISP_MGR_NSL2_DEV<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  ISP_MGR_NSL2_DEV<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  ISP_MGR_NSL2_DEV<ESensorDev_Sub>::getInstance();
    default:
        MY_ERR("eSensorDev = %d", eSensorDev);
        return  ISP_MGR_NSL2_DEV<ESensorDev_Main>::getInstance();
    }
}

template <>
ISP_MGR_NSL2_T&
ISP_MGR_NSL2_T::
put(ISP_NVRAM_NSL2_T const& rParam)
{

    PUT_REG_INFO(CAM_NSL2_CEN, cen);
    PUT_REG_INFO(CAM_NSL2_RR_CON0, rr_con0);
    PUT_REG_INFO(CAM_NSL2_RR_CON1, rr_con1);
    PUT_REG_INFO(CAM_NSL2_GAIN, gain);
    //Evalutaing slope

    //Comparing which corner is the longest distance, and use it as R.

	//corner1
    int SL2_CENTR_Y = rParam.cen.bits.SL2_CENTR_X;
    int SL2_CENTR_X = rParam.cen.bits.SL2_CENTR_X;
    int NSL2_width = m_NSL2width;
    int NSL2_height = m_NSL2height;
	int y_dist_corner1 = (0 - SL2_CENTR_Y);
	int x_dist_corner1 = (0 - SL2_CENTR_X);
	int CircleEq_corner1;

	if(y_dist_corner1 <0 )
		y_dist_corner1 = -y_dist_corner1;

	if(x_dist_corner1 <0 )
		x_dist_corner1 = -x_dist_corner1;


	if(x_dist_corner1 >= y_dist_corner1)
		CircleEq_corner1 = x_dist_corner1;
	else
		CircleEq_corner1 = y_dist_corner1;

	int tmp_corner1 = ((x_dist_corner1 + y_dist_corner1)*1448) >> 11;

	if(tmp_corner1 >= CircleEq_corner1)
		CircleEq_corner1 = tmp_corner1;

	//corner 2
	int y_dist_corner2 = (0 - SL2_CENTR_Y);
	int x_dist_corner2 = (NSL2_width - SL2_CENTR_X);
	int CircleEq_corner2;

	if(y_dist_corner2 <0 )
		y_dist_corner2 = -y_dist_corner2;

	if(x_dist_corner2 <0 )
		x_dist_corner2 = -x_dist_corner2;


	if(x_dist_corner2 >= y_dist_corner2)
		CircleEq_corner2 = x_dist_corner2;
	else
		CircleEq_corner2 = y_dist_corner2;

	int tmp_corner2 = ((x_dist_corner2 + y_dist_corner2)*1448) >> 11;

	if(tmp_corner2 >= CircleEq_corner2)
		CircleEq_corner2 = tmp_corner2;



	//corner 3
	int y_dist_corner3 = (NSL2_height - SL2_CENTR_Y);
	int x_dist_corner3 = (0 - SL2_CENTR_X);
	int CircleEq_corner3;

	if(y_dist_corner3 <0 )
		y_dist_corner3 = -y_dist_corner3;

	if(x_dist_corner3 <0 )
		x_dist_corner3 = -x_dist_corner3;


	if(x_dist_corner3 >= y_dist_corner3)
		CircleEq_corner3 = x_dist_corner3;
	else
		CircleEq_corner3 = y_dist_corner3;

	int tmp_corner3 = ((x_dist_corner3 + y_dist_corner3)*1448) >> 11;

	if(tmp_corner3 >= CircleEq_corner3)
		CircleEq_corner3 = tmp_corner3;



	//corner4
	int y_dist_corner4 = (NSL2_height - SL2_CENTR_Y);
	int x_dist_corner4 = (NSL2_width - SL2_CENTR_X);
	int CircleEq_corner4;

	if(y_dist_corner4 <0 )
		y_dist_corner4 = -y_dist_corner4;

	if(x_dist_corner4 <0 )
		x_dist_corner4 = -x_dist_corner4;


	if(x_dist_corner4 >= y_dist_corner4)
		CircleEq_corner4 = x_dist_corner4;
	else
		CircleEq_corner4 = y_dist_corner4;

	int tmp_corner4 = ((x_dist_corner4 + y_dist_corner4)*1448) >> 11;

	if(tmp_corner4 >= CircleEq_corner4)
		CircleEq_corner4 = tmp_corner4;



	int max;
	if(CircleEq_corner4 > CircleEq_corner3)
		max = CircleEq_corner4;
	else
		max = CircleEq_corner3;

	if(CircleEq_corner2 > max)
		max = CircleEq_corner2;

	if(CircleEq_corner1 > max)
		max = CircleEq_corner1;



    int tempGain0 = rParam.rr_con1.bits.SL2_GAIN_0;
    int tempGain1 = rParam.rr_con1.bits.SL2_GAIN_1;
    int tempGain2 = rParam.gain.bits.SL2_GAIN_2;
    int tempGain3 = rParam.gain.bits.SL2_GAIN_3;
    int tempGain4 = rParam.gain.bits.SL2_GAIN_4;
    int tempR0 = rParam.rr_con0.bits.SL2_R_0;
    int tempR1 = rParam.rr_con0.bits.SL2_R_1;
    int tempR2 = rParam.rr_con1.bits.SL2_R_2;

	int SL2_SLP_1  = (int)(((float)(tempGain1 - tempGain0)/(float)tempR0)*SLP_PREC_F_SCAL + 0.5) ;
	int SL2_SLP_2  = (int)(((float)(tempGain2 - tempGain1)/(float)(tempR1 - tempR0))*SLP_PREC_F_SCAL + 0.5) ;
	int SL2_SLP_3  = (int)(((float)(tempGain3 - tempGain2)/(float)(tempR2 - tempR1))*SLP_PREC_F_SCAL + 0.5) ;
	int SL2_SLP_4  = (int)(((float)(tempGain4 - tempGain3)/(float)(max -    tempR2))*SLP_PREC_F_SCAL + 0.5) ;

    CAM_REG_NSL2_SLP_CON0 rSlopCon0;
    CAM_REG_NSL2_SLP_CON1 rSlopCon1;
    rSlopCon0.Bits.SL2_SLP_1 = SL2_SLP_1;
    rSlopCon0.Bits.SL2_SLP_2 = SL2_SLP_2;
    rSlopCon1.Bits.SL2_SLP_3 = SL2_SLP_3;
    rSlopCon1.Bits.SL2_SLP_4 = SL2_SLP_4;


    REG_INFO_VALUE (CAM_NSL2_SLP_CON0) = rSlopCon0.Raw;
    REG_INFO_VALUE (CAM_NSL2_SLP_CON1) = rSlopCon1.Raw;




    return  (*this);
}


template <>
ISP_MGR_NSL2_T&
ISP_MGR_NSL2_T::
get(ISP_NVRAM_NSL2_T& rParam)
{
    GET_REG_INFO(CAM_NSL2_CEN, cen);
    GET_REG_INFO(CAM_NSL2_RR_CON0, rr_con0);
    GET_REG_INFO(CAM_NSL2_RR_CON1, rr_con1);
    GET_REG_INFO(CAM_NSL2_GAIN, gain);


    return  (*this);
}

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

MBOOL
ISP_MGR_NSL2_T::
apply(EIspProfile_T eIspProfile)
{
    //setEnable(MTRUE);    //CCTenable?

    addressErrorCheck("Before ISP_MGR_NSL2_T::apply()");

    MBOOL bNSL2_EN = isEnable();
    ESoftwareScenario eSwScn = static_cast<ESoftwareScenario>(m_rIspDrvScenario[eIspProfile]);

    MINT32 i4Flg = 0;
    GET_PROP("debug.nsl2.en", "7", i4Flg);
    MUINT32 u4NSL2_EN =  (bNSL2_EN&&(i4Flg&1)) ? 1 : 0;
    MUINT32 u4NSL2a_EN = (bNSL2_EN&&(i4Flg&2)) ? 1 : 0;
    MUINT32 u4NSL2b_EN = (bNSL2_EN&&(i4Flg&2)) ? 1 : 0;
    MUINT32 u4SL2c_EN =  (bNSL2_EN&&(i4Flg&4)) ? 1 : 0;

    #if 1
    if (eIspProfile == EIspProfile_VFB_PostProc ||
            eIspProfile == EIspProfile_Capture_MultiPass_ANR_1 ||
            eIspProfile == EIspProfile_Capture_MultiPass_ANR_2 ||
            eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
            eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2 ||
            eIspProfile == EIspProfile_MFB_MultiPass_ANR_1          ||
            eIspProfile == EIspProfile_MFB_MultiPass_ANR_2          ||
            eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR    ||
            eIspProfile == EIspProfile_MFB_Blending_All_Off         ||
            eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR ||
            eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off ||
            eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR     ||
            eIspProfile == EIspProfile_MFB_PostProc_Mixing          ||
            eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_1          ||
            eIspProfile == EIspProfile_VSS_MFB_MultiPass_ANR_2          ||
            eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR     ||
            eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
    )
    #else
    if (eIspProfile == EIspProfile_VFB_PostProc ||
        eIspProfile == EIspProfile_Capture_MultiPass_ANR_1 ||
        eIspProfile == EIspProfile_Capture_MultiPass_ANR_2 ||
        eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
        eIspProfile == EIspProfile_VSS_Capture_MultiPass_ANR_2 ||
        eIspProfile == EIspProfile_MFB_MultiPass_ANR_1 ||
        eIspProfile == EIspProfile_MFB_MultiPass_ANR_2 ||
        eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR ||
        eIspProfile == EIspProfile_MFB_Blending_All_Off ||
        eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR ||
        eIspProfile == EIspProfile_MFB_PostProc_Mixing )
    #endif
    {
        u4NSL2_EN = 0;
        u4NSL2a_EN = 0;
        u4NSL2b_EN = 0;
        u4SL2c_EN =  0;
    }

    ISP_MGR_CTL_EN_P2_T::getInstance(m_eSensorDev).setEnable_NSL2(u4NSL2_EN);
    //m_bEnable = u4NSL2_EN ? MTRUE : MFALSE;
    m_bNSL2AOnOff = u4NSL2a_EN ? MTRUE : MFALSE;
    m_bNSL2BOnOff = u4NSL2b_EN ? MTRUE : MFALSE;

    REG_INFO_VALUE(CAM_NSL2_RZ) = 0x08000800;    //VRZ & HRZ
    REG_INFO_VALUE(CAM_NSL2_XOFF) = 0;
    REG_INFO_VALUE(CAM_NSL2_YOFF) = 0;

    // TOP
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Nsl2);
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Nsl2a);
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Nsl2b);
    TuningMgr::getInstance().updateEngine(eSwScn, eTuningMgrFunc_Sl2c);



	TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, NSL2_EN, u4NSL2_EN);
    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, NSL2A_EN, u4NSL2a_EN);
    TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, NSL2B_EN, u4NSL2b_EN);
	TUNING_MGR_WRITE_ENABLE_BITS(eSwScn, CAM_CTL_EN_P2, SL2C_EN, u4SL2c_EN);

    // Register setting
    TuningMgr::getInstance().tuningMgrWriteRegs(eSwScn, static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum);

   // MY_LOG_IF(1/*ENABLE_MY_LOG*/,
   //     "[%s] eSwScn(%d), bNSL2_EN(%d), u4NSL2_EN(%d), u4NSL2b_EN(%d), u4SL2c_EN(%d), CAM_NSL2_CEN(0x%08x), CAM_NSL2_MAX0_RR(0x%08x), CAM_NSL2_MAX1_RR(0x%08x), CAM_NSL2_MAX2_RR(0x%08x)",
    //    __FUNCTION__,
   //     eSwScn, bNSL2_EN, u4NSL2_EN, u4NSL2b_EN, u4SL2c_EN,
   //     REG_INFO_VALUE(CAM_NSL2_CEN),
   //     REG_INFO_VALUE(CAM_NSL2_MAX0_RR),
   //     REG_INFO_VALUE(CAM_NSL2_MAX1_RR),
   //     REG_INFO_VALUE(CAM_NSL2_MAX2_RR));

    dumpRegInfo("NSL2");

    addressErrorCheck("After ISP_MGR_NSL2_T::apply()");

    return  MTRUE;
}


}
