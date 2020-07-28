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

#define LOG_TAG "isp_tuning_user"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
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
#include <math.h>

#include "camera_custom_lomo_param.h" 
#include "camera_custom_lomo_param_jni.h" 

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EE
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::userSetting_EE(
    RAWIspCamInfo const& rCamInfo,
    EIndex_Isp_Edge_T eIdx_Edge,
    ISP_NVRAM_EE_T& rEE
)
{
    MINT32 i4Slider, dX, dY;

    MY_LOG("eIdx_Edge : %d\n",eIdx_Edge);

    switch ( eIdx_Edge )
    {
    case MTK_CONTROL_ISP_EDGE_LOW:
        i4Slider = -4; // Soft EE
        break;
    case MTK_CONTROL_ISP_EDGE_HIGH:
        i4Slider = 7; //Texture EE
        break;
    case MTK_CONTROL_ISP_EDGE_MIDDLE: // Normal EE
    default:
        i4Slider = 0;
        break;
    }

    if (i4Slider == 0) // Normal EE: do nothing
        return;

    // Get normal EE parameter
    MINT32 LOCK_EE_Y1 = rEE.glut_ctrl_01.bits.SEEE_GLUT_Y1;   // 0~1023
	MINT32 LOCK_EE_Y2 = rEE.glut_ctrl_02.bits.SEEE_GLUT_Y2;   // 0~1023
	MINT32 LOCK_EE_Y3 = rEE.glut_ctrl_03.bits.SEEE_GLUT_Y3;   // 0~1023
	MINT32 LOCK_EE_Y4 = rEE.glut_ctrl_04.bits.SEEE_GLUT_Y4;   // 0~1023
	MINT32 LOCK_EE_S5 = rEE.glut_ctrl_05.bits.SEEE_GLUT_S5;   // 0~255
	MINT32 LOCK_EE_CLIP = rEE.clip_ctrl.bits.SEEE_RESP_CLIP; // 0~255
	MINT32 LOCK_EE_TH_OV = rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR; // 0~255
	MINT32 LOCK_EE_TH_UN = rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND; // 0~255
	MINT32 USM_ED_Y1, USM_ED_Y2, USM_ED_Y3, USM_ED_Y4, USM_CLIP, USM_ED_TH_OVER, USM_ED_TH_UNDER;

    // Determine user setting EE parameter
    if (i4Slider < 0) {  // Soft EE
        USM_ED_Y1 = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_Y1) * (1 + 0.5 * i4Slider) + 0.5), 0, 1023);
        USM_ED_Y2 = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_Y2) * (1 + 0.3 * i4Slider) + 0.5), 0, 1023);
    }
    else { // Texture EE
        USM_ED_Y1 = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_Y1) * (1 + 1.0 * i4Slider) + 0.5), 0, 1023);
        USM_ED_Y2 = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_Y2) * (1 + 1.5 * i4Slider) + 0.5), 0, 1023);
    }

	USM_ED_Y3 = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_Y3) * (1 + 0.2 * i4Slider) + 0.5), 0, 1023);
	USM_ED_Y4 = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_Y4) * (1 + 0.2 * i4Slider) + 0.5), 0, 1023);
	USM_CLIP = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_CLIP) * (1 + 0.2 * i4Slider) + 0.5), 0, 255);
	USM_ED_TH_OVER = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_TH_OV) * (1 + 0.2 * i4Slider) + 0.5), 0, 255);
	USM_ED_TH_UNDER = LIMIT(static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_TH_UN) * (1 + 0.2 * i4Slider) + 0.5), 0, 255);

	MINT32 EE_LCE_THO_1 = LIMIT(static_cast<MINT32>(USM_ED_TH_OVER - 10), 0, 255);
	MINT32 EE_LCE_THO_2 = LIMIT(static_cast<MINT32>(USM_ED_TH_OVER - 20), 0, 255);
	MINT32 EE_LCE_THO_3 = LIMIT(static_cast<MINT32>(USM_ED_TH_OVER - 30), 0, 255);
	MINT32 EE_LCE_THU_1 = LIMIT(static_cast<MINT32>(USM_ED_TH_UNDER - 10), 0, 255);
	MINT32 EE_LCE_THU_2 = LIMIT(static_cast<MINT32>(USM_ED_TH_UNDER - 20), 0, 255);
	MINT32 EE_LCE_THU_3 = LIMIT(static_cast<MINT32>(USM_ED_TH_UNDER - 30), 0, 255);

    // USM_ED_S1
    MINT32 USM_ED_X1 = rEE.glut_ctrl_01.bits.SEEE_GLUT_X1;
    MINT32 USM_ED_S1;

    if (USM_ED_X1 == 0) USM_ED_S1 = 0;
    else USM_ED_S1 = static_cast<MINT32>(static_cast<MDOUBLE>(USM_ED_Y1) / USM_ED_X1 + 0.5);

    if (USM_ED_S1 > 127) USM_ED_S1 = 127;

    // EE_LCE_S1_1
    MINT32 EE_LCE_X1_1 = rEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1;
    MINT32 EE_LCE_S1_1;

    if (EE_LCE_X1_1 == 0) EE_LCE_S1_1 = 0;
    else EE_LCE_S1_1 = static_cast<MINT32>(static_cast<MDOUBLE>(USM_ED_Y1) / EE_LCE_X1_1 + 0.5);

    if (EE_LCE_S1_1 > 127) EE_LCE_S1_1 = 127;

    // EE_LCE_S1_2
    MINT32 EE_LCE_X1_2 = rEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2;
    MINT32 EE_LCE_S1_2;

    if (EE_LCE_X1_2 == 0) EE_LCE_S1_2 = 0;
    else EE_LCE_S1_2 = static_cast<MINT32>(static_cast<MDOUBLE>(USM_ED_Y1) / EE_LCE_X1_2 + 0.5);

    if(EE_LCE_S1_2 > 127) EE_LCE_S1_2 = 127;

    // EE_LCE_S1_3
    MINT32 EE_LCE_X1_3 = rEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3;
    MINT32 EE_LCE_S1_3;

    if (EE_LCE_X1_3 == 0) EE_LCE_S1_3 = 0;
    else EE_LCE_S1_3 = static_cast<MINT32>(static_cast<MDOUBLE>(USM_ED_Y1) / EE_LCE_X1_3 + 0.5);

    if (EE_LCE_S1_3 > 127) EE_LCE_S1_3 = 127;

    // USM_ED_S2
    MINT32 USM_ED_X2 = rEE.glut_ctrl_02.bits.SEEE_GLUT_X2;
    MINT32 USM_ED_S2;
    // EE_LCE_S2_1
    MINT32 EE_LCE_S2_1;
    // EE_LCE_S2_2
    MINT32 EE_LCE_S2_2;
    // EE_LCE_S2_3
    MINT32 EE_LCE_S2_3;

    dY = USM_ED_Y2 - USM_ED_Y1;
    if (dY > 0){
        // USM_ED_S2
        dX = USM_ED_X2 - USM_ED_X1;
        if (dX == 0) USM_ED_S2 = 0;
        else USM_ED_S2 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX + 0.5);

        // EE_LCE_S2_1
        dX = USM_ED_X2 - EE_LCE_X1_1;
        if (dX == 0) EE_LCE_S2_1 = 0;
        else EE_LCE_S2_1 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX + 0.5);

        // EE_LCE_S2_2
        dX = USM_ED_X2 - EE_LCE_X1_2;
        if (dX == 0) EE_LCE_S2_2 = 0;
        else EE_LCE_S2_2 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX + 0.5);

        // EE_LCE_S2_3
        dX = USM_ED_X2 - EE_LCE_X1_3;
        if (dX == 0) EE_LCE_S2_3 = 0;
        else EE_LCE_S2_3 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX + 0.5);
    }
    else {
        // USM_ED_S2
        dX = USM_ED_X2 - USM_ED_X1;
        if (dX == 0) USM_ED_S2 = 0;
        else USM_ED_S2 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX - 0.5);

        // EE_LCE_S2_1
        dX = USM_ED_X2 - EE_LCE_X1_1;
        if (dX == 0) EE_LCE_S2_1 = 0;
        else EE_LCE_S2_1 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX  - 0.5);

        // EE_LCE_S2_2
        dX = USM_ED_X2 - EE_LCE_X1_2;
        if (dX == 0) EE_LCE_S2_2 = 0;
        else EE_LCE_S2_2 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX - 0.5);

        // EE_LCE_S2_3
        dX = USM_ED_X2 - EE_LCE_X1_3;
        if (dX == 0) EE_LCE_S2_3 = 0;
        else EE_LCE_S2_3 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX - 0.5);
    }

    USM_ED_S2 = LIMIT(USM_ED_S2, -127, 127);
    EE_LCE_S2_1 = LIMIT(EE_LCE_S2_1, -127, 127);
    EE_LCE_S2_2 = LIMIT(EE_LCE_S2_2, -127, 127);
    EE_LCE_S2_3 = LIMIT(EE_LCE_S2_3, -127, 127);

    // USM_ED_S3
    MINT32 USM_ED_X3 = rEE.glut_ctrl_03.bits.SEEE_GLUT_X3;
    MINT32 USM_ED_S3;
    dX = USM_ED_X3 - USM_ED_X2;
    dY = USM_ED_Y3 - USM_ED_Y2;
    if (dY > 0) {
        if (dX == 0) USM_ED_S3 = 0;
        else USM_ED_S3 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX + 0.5);
    }
    else {
        if (dX == 0) USM_ED_S3 = 0;
        else USM_ED_S3 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX - 0.5);
    }

    USM_ED_S3 = LIMIT(USM_ED_S3, -127, 127);

    // USM_ED_S4
    MINT32 USM_ED_X4 = rEE.glut_ctrl_04.bits.SEEE_GLUT_X4;
    MINT32 USM_ED_S4;
    dX = USM_ED_X4 - USM_ED_X3;
    dY = USM_ED_Y4 - USM_ED_Y3;
    if (dY > 0){
        if (dX == 0) USM_ED_S4 = 0;
        else USM_ED_S4 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX + 0.5);
    }
    else{
        if (dX == 0) USM_ED_S4 = 0;
        else USM_ED_S4 = static_cast<MINT32>(static_cast<MDOUBLE>(dY) / dX - 0.5);
    }

    USM_ED_S4 = LIMIT(USM_ED_S4, -127, 127);

    // USM_ED_S5
	MINT32 USM_ED_S5;
	dX = 255 - USM_ED_X4;
	if (LOCK_EE_S5 < 128){
		if (dX == 0) USM_ED_S5 = 0;
		else USM_ED_S5 = static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_S5) * (1 + 0.2 * i4Slider) + 0.5);
    }
    else {
		if (dX == 0) USM_ED_S5 = 0;
		else USM_ED_S5 = static_cast<MINT32>(static_cast<MDOUBLE>(LOCK_EE_S5 - 256) * (1 + 0.2 * i4Slider) - 0.5);
    }

    USM_ED_S5 = LIMIT(USM_ED_S5, -127, 127);


    MY_LOG("[userSetting_EE] old\nX1, Y1, S1: %3d, %3d, %3d, 0x%08x\nX2, Y2, S2: %3d, %3d, %3d, 0x%08x\nX3, Y3, S3: %3d, %3d, %3d, 0x%08x\nX4, Y4, S4: %3d, %3d, %3d, 0x%08x\nS5: %3d, 0x%08x\nCLIP: %3d, 0x%08x\nTHO, THU: %3d, %3d, 0x%08x\n",
                               rEE.glut_ctrl_01.bits.SEEE_GLUT_X1,
                               rEE.glut_ctrl_01.bits.SEEE_GLUT_Y1,
                               rEE.glut_ctrl_01.bits.SEEE_GLUT_S1,
							   rEE.glut_ctrl_01.val,
                               rEE.glut_ctrl_02.bits.SEEE_GLUT_X2,
                               rEE.glut_ctrl_02.bits.SEEE_GLUT_Y2,
                               rEE.glut_ctrl_02.bits.SEEE_GLUT_S2,
							   rEE.glut_ctrl_02.val,
                               rEE.glut_ctrl_03.bits.SEEE_GLUT_X3,
                               rEE.glut_ctrl_03.bits.SEEE_GLUT_Y3,
                               rEE.glut_ctrl_03.bits.SEEE_GLUT_S3,
							   rEE.glut_ctrl_03.val,
                               rEE.glut_ctrl_04.bits.SEEE_GLUT_X4,
                               rEE.glut_ctrl_04.bits.SEEE_GLUT_Y4,
                               rEE.glut_ctrl_04.bits.SEEE_GLUT_S4,
							   rEE.glut_ctrl_04.val,
                               rEE.glut_ctrl_05.bits.SEEE_GLUT_S5,
							   rEE.glut_ctrl_05.val,
							   rEE.clip_ctrl.bits.SEEE_RESP_CLIP,
							   rEE.clip_ctrl.val,
							   rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR,
							   rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND,
							   rEE.glut_ctrl_06.val
                               );

    MY_LOG("[userSetting_EE] old\nLCE_X1_1, LCE_S1_1, LCE_S2_1: %3d, %3d, %3d, 0x%08x\nLCE_X1_2, LCE_S1_2, LCE_S2_2: %3d, %3d, %3d, 0x%08x\nLCE_X1_3, LCE_S1_3, LCE_S2_3: %3d, %3d, %3d, 0x%08x\nLCE_THO_1, LCE_THU_1, LCE_THO_2, LCE_THU_2: %3d, %3d, %3d, %3d, 0x%08x\nLCE_THO_3, LCE_THU_3: %3d, %3d, 0x%08x\n",
                               rEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1,
                               rEE.glut_ctrl_07.bits.SEEE_GLUT_S1_1,
                               rEE.glut_ctrl_07.bits.SEEE_GLUT_S2_1,
							   rEE.glut_ctrl_07.val,
                               rEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2,
                               rEE.glut_ctrl_08.bits.SEEE_GLUT_S1_2,
                               rEE.glut_ctrl_08.bits.SEEE_GLUT_S2_2,
							   rEE.glut_ctrl_08.val,
                               rEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3,
                               rEE.glut_ctrl_09.bits.SEEE_GLUT_S1_3,
                               rEE.glut_ctrl_09.bits.SEEE_GLUT_S2_3,
							   rEE.glut_ctrl_09.val,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2,
							   rEE.glut_ctrl_10.val,
							   rEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3,
							   rEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3,
							   rEE.glut_ctrl_11.val
                               );

    // Write back
    rEE.glut_ctrl_01.bits.SEEE_GLUT_Y1 = static_cast<MUINT32>(USM_ED_Y1);
    rEE.glut_ctrl_02.bits.SEEE_GLUT_Y2 = static_cast<MUINT32>(USM_ED_Y2);
    rEE.glut_ctrl_03.bits.SEEE_GLUT_Y3 = static_cast<MUINT32>(USM_ED_Y3);
    rEE.glut_ctrl_04.bits.SEEE_GLUT_Y4 = static_cast<MUINT32>(USM_ED_Y4);
    rEE.clip_ctrl.bits.SEEE_RESP_CLIP = static_cast<MUINT32>(USM_CLIP);
    rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR = static_cast<MUINT32>(USM_ED_TH_OVER);
    rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND = static_cast<MUINT32>(USM_ED_TH_UNDER);

    rEE.glut_ctrl_01.bits.SEEE_GLUT_S1 = (USM_ED_S1 >= 0) ? static_cast<MUINT32>(USM_ED_S1) : static_cast<MUINT32>(256 + USM_ED_S1);
    rEE.glut_ctrl_02.bits.SEEE_GLUT_S2 = (USM_ED_S2 >= 0) ? static_cast<MUINT32>(USM_ED_S2) : static_cast<MUINT32>(256 + USM_ED_S2);
    rEE.glut_ctrl_03.bits.SEEE_GLUT_S3 = (USM_ED_S3 >= 0) ? static_cast<MUINT32>(USM_ED_S3) : static_cast<MUINT32>(256 + USM_ED_S3);
    rEE.glut_ctrl_04.bits.SEEE_GLUT_S4 = (USM_ED_S4 >= 0) ? static_cast<MUINT32>(USM_ED_S4) : static_cast<MUINT32>(256 + USM_ED_S4);
    rEE.glut_ctrl_05.bits.SEEE_GLUT_S5 = (USM_ED_S5 >= 0) ? static_cast<MUINT32>(USM_ED_S5) : static_cast<MUINT32>(256 + USM_ED_S5);

    rEE.glut_ctrl_07.bits.SEEE_GLUT_S1_1 = (EE_LCE_S1_1 >= 0) ? static_cast<MUINT32>(EE_LCE_S1_1) : static_cast<MUINT32>(256 + EE_LCE_S1_1);
    rEE.glut_ctrl_07.bits.SEEE_GLUT_S2_1 = (EE_LCE_S2_1 >= 0) ? static_cast<MUINT32>(EE_LCE_S2_1) : static_cast<MUINT32>(256 + EE_LCE_S2_1);
    rEE.glut_ctrl_08.bits.SEEE_GLUT_S1_2 = (EE_LCE_S1_2 >= 0) ? static_cast<MUINT32>(EE_LCE_S1_2) : static_cast<MUINT32>(256 + EE_LCE_S1_2);
    rEE.glut_ctrl_08.bits.SEEE_GLUT_S2_2 = (EE_LCE_S2_2 >= 0) ? static_cast<MUINT32>(EE_LCE_S2_2) : static_cast<MUINT32>(256 + EE_LCE_S2_2);
    rEE.glut_ctrl_09.bits.SEEE_GLUT_S1_3 = (EE_LCE_S1_3 >= 0) ? static_cast<MUINT32>(EE_LCE_S1_3) : static_cast<MUINT32>(256 + EE_LCE_S1_3);
    rEE.glut_ctrl_09.bits.SEEE_GLUT_S2_3 = (EE_LCE_S2_3 >= 0) ? static_cast<MUINT32>(EE_LCE_S2_3) : static_cast<MUINT32>(256 + EE_LCE_S2_3);

	rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1 = EE_LCE_THO_1;
	rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2 = EE_LCE_THO_2;
	rEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3 = EE_LCE_THO_3;
	rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1 = EE_LCE_THU_1;
	rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2 = EE_LCE_THU_2;
	rEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3 = EE_LCE_THU_3;

	MY_LOG("[userSetting_EE] new\nX1, Y1, S1: %3d, %3d, %3d, 0x%08x\nX2, Y2, S2: %3d, %3d, %3d, 0x%08x\nX3, Y3, S3: %3d, %3d, %3d, 0x%08x\nX4, Y4, S4: %3d, %3d, %3d, 0x%08x\nS5: %3d, 0x%08x\nCLIP: %3d, 0x%08x\nTHO, THU: %3d, %3d, 0x%08x\n",
                               rEE.glut_ctrl_01.bits.SEEE_GLUT_X1,
                               rEE.glut_ctrl_01.bits.SEEE_GLUT_Y1,
                               rEE.glut_ctrl_01.bits.SEEE_GLUT_S1,
							   rEE.glut_ctrl_01.val,
                               rEE.glut_ctrl_02.bits.SEEE_GLUT_X2,
                               rEE.glut_ctrl_02.bits.SEEE_GLUT_Y2,
                               rEE.glut_ctrl_02.bits.SEEE_GLUT_S2,
							   rEE.glut_ctrl_02.val,
                               rEE.glut_ctrl_03.bits.SEEE_GLUT_X3,
                               rEE.glut_ctrl_03.bits.SEEE_GLUT_Y3,
                               rEE.glut_ctrl_03.bits.SEEE_GLUT_S3,
							   rEE.glut_ctrl_03.val,
                               rEE.glut_ctrl_04.bits.SEEE_GLUT_X4,
                               rEE.glut_ctrl_04.bits.SEEE_GLUT_Y4,
                               rEE.glut_ctrl_04.bits.SEEE_GLUT_S4,
							   rEE.glut_ctrl_04.val,
                               rEE.glut_ctrl_05.bits.SEEE_GLUT_S5,
							   rEE.glut_ctrl_05.val,
							   rEE.clip_ctrl.bits.SEEE_RESP_CLIP,
							   rEE.clip_ctrl.val,
							   rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_OVR,
							   rEE.glut_ctrl_06.bits.SEEE_GLUT_TH_UND,
							   rEE.glut_ctrl_06.val
                               );

	MY_LOG("[userSetting_EE] new\nLCE_X1_1, LCE_S1_1, LCE_S2_1: %3d, %3d, %3d, 0x%08x\nLCE_X1_2, LCE_S1_2, LCE_S2_2: %3d, %3d, %3d, 0x%08x\nLCE_X1_3, LCE_S1_3, LCE_S2_3: %3d, %3d, %3d, 0x%08x\nLCE_THO_1, LCE_THU_1, LCE_THO_2, LCE_THU_2: %3d, %3d, %3d, %3d, 0x%08x\nLCE_THO_3, LCE_THU_3: %3d, %3d, 0x%08x\n",
                               rEE.glut_ctrl_07.bits.SEEE_GLUT_X1_1,
                               rEE.glut_ctrl_07.bits.SEEE_GLUT_S1_1,
                               rEE.glut_ctrl_07.bits.SEEE_GLUT_S2_1,
							   rEE.glut_ctrl_07.val,
                               rEE.glut_ctrl_08.bits.SEEE_GLUT_X1_2,
                               rEE.glut_ctrl_08.bits.SEEE_GLUT_S1_2,
                               rEE.glut_ctrl_08.bits.SEEE_GLUT_S2_2,
							   rEE.glut_ctrl_08.val,
                               rEE.glut_ctrl_09.bits.SEEE_GLUT_X1_3,
                               rEE.glut_ctrl_09.bits.SEEE_GLUT_S1_3,
                               rEE.glut_ctrl_09.bits.SEEE_GLUT_S2_3,
							   rEE.glut_ctrl_09.val,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_1,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_1,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_OVR_2,
							   rEE.glut_ctrl_10.bits.SEEE_GLUT_TH_UND_2,
							   rEE.glut_ctrl_10.val,
							   rEE.glut_ctrl_11.bits.SEEE_GLUT_TH_OVR_3,
							   rEE.glut_ctrl_11.bits.SEEE_GLUT_TH_UND_3,
							   rEE.glut_ctrl_11.val
                               );

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// HSBC + Effect
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define PI (3.1415926)
#define max(x1,x2) ((x1) > (x2) ? (x1):(x2))
#define Round(dInput) ((dInput) > 0.0f ? (int)(dInput + 0.5f):(int)(dInput - 0.5f))

MVOID
IspTuningCustom::userSetting_EFFECT(
        RAWIspCamInfo const& rCamInfo, 
        EIndex_Effect_T const& eIdx_Effect, 
        IspUsrSelectLevel_T const& rIspUsrSelectLevel, 
        ISP_NVRAM_G2C_T& rG2C, 
        ISP_NVRAM_G2C_SHADE_T& rG2C_SHADE,
        ISP_NVRAM_SE_T& rSE,
        ISP_NVRAM_GGM_T& rGGM
)
{
	MDOUBLE H_param[] = {-40, 0, 40};				// rotate degree H of 360
	MDOUBLE S_param[] = {44.0/64, 1, 84.0/64};		// gain = S
	MDOUBLE B_param[] = {-16*4, 0, 16*4};			// add B of 256
	MDOUBLE C_param[] = { 54.0/64, 1, 74.0/64};		// gain = C
	MINT32 M00, M01, M02, M10, M11, M12, M20, M21, M22, Yoffset, Uoffset, Voffset;
	MDOUBLE H, S, B, C;
	MDOUBLE V=1;
	MDOUBLE Cb=0;
	MDOUBLE Cr=0;
    MINT32 YR,YG, YB, UR, UG, UB, VR, VG, VB, Y_OFFSET11, U_OFFSET10, V_OFFSET10;		// ISP registers for RGB2YUV
    MUINT32 SE_EDGE, SE_Y, SE_Y_CONST, SE_OILEN, SE_KNEESEL, SE_EGAIN_HB, SE_EGAIN_HA,	     \
	        SE_SPECIPONLY, SE_SPECIGAIN, SE_EGAIN_VB, SE_EGAIN_VA, SE_SPECIABS,	SE_SPECIINV, \
	        SE_COREH, SE_E_TH1_V, SE_EMBOSS1, SE_EMBOSS2, SE_YOUT_QBIT, SE_COUT_QBIT;	// ISP registers for SEEE
    MUINT32 i;

	M00 = 153;
	M01 = 301;
	M02 = 58;
	M10 = -86;
	M11 = -170;
	M12 = 256;
	M20 = 256;
	M21 = -214;
	M22 = -42;
	Yoffset = 0;
	Uoffset = 0;
	Voffset = 0;

    SE_Y = SE_Y_CONST = SE_OILEN = SE_KNEESEL = SE_EGAIN_HB = SE_EGAIN_HA = \
	SE_SPECIPONLY = SE_SPECIGAIN = SE_EGAIN_VB = SE_EGAIN_VA = SE_SPECIABS = SE_SPECIINV = \
	SE_COREH = SE_E_TH1_V = SE_EMBOSS1 = SE_EMBOSS2 = SE_YOUT_QBIT = SE_COUT_QBIT = 0;

    SE_EDGE = 1;

	MY_LOG("special effect selection : %d\n",eIdx_Effect);
	MY_LOG("input user level : H:%d S:%d B:%d C:%d\n",rIspUsrSelectLevel.eIdx_Hue,
                                                      rIspUsrSelectLevel.eIdx_Sat,
                                                      rIspUsrSelectLevel.eIdx_Bright,
                                                      rIspUsrSelectLevel.eIdx_Contrast);
    #ifdef MTK_LOMO_SUPPORT
	MY_LOG("MTK_LOMO_SUPPORT %d\n",MTK_LOMO_SUPPORT);
    #else
	MY_LOG(" !MTK_LOMO_SUPPORT XX\n");
	MY_LOG("MTK_LOMO_SUPPORT %d\n",MTK_LOMO_SUPPORT);
    #endif
    #ifdef MTK_CAM_LOMO_SUPPORT
	MY_LOG("MTK_CAM__LOMO_SUPPORT %d\n",MTK_LOMO_SUPPORT);
    #else
	MY_LOG(" !MTK_CAM__LOMO_SUPPORT XX\n");

    #endif

	switch (eIdx_Effect) {
		case MTK_CONTROL_EFFECT_MODE_MONO: // Mono
			M10 = 0;
			M11 = 0;
			M12 = 0;
			M20 = 0;
			M21 = 0;
			M22 = 0;

			//SE_EDGE = 0;

            break;

		case MTK_CONTROL_EFFECT_MODE_SEPIA:	// Sepia
			M10 = 0;
			M11 = 0;
			M12 = 0;
			M20 = 0;
			M21 = 0;
			M22 = 0;
			Uoffset = -120; // -72 (Recommend)
			Voffset =  120; //  88 (Recommend)

			//SE_EDGE = 0;

			break;

		case MTK_CONTROL_EFFECT_MODE_AQUA: // Aqua
			M10 = 0;
			M11 = 0;
			M12 = 0;
			M20 = 0;
			M21 = 0;
			M22 = 0;
			Uoffset = 352;	//  154 (Recommend)
			Voffset = -120;	// -154 (Recommend)

			//SE_EDGE = 0;

			break;

		case MTK_CONTROL_EFFECT_MODE_NEGATIVE: // Negative
			M00 = -M00;
			M01 = -M01;
			M02 = -M02;
			M10 = -M10;
			M11 = -M11;
			M12 = -M12;
			M20 = -M20;
			M21 = -M21;
			M22 = -M22;
			Yoffset = 1023;

			//SE_EDGE = 0;

			break;

		case MTK_CONTROL_EFFECT_MODE_POSTERIZE: // Posterize

			M00 = M00*1.6;
			M01 = M01*1.6;
			M02 = M02*1.6;
			M10 = M10*1.6;
			M11 = M11*1.6;
			M12 = M12*1.6;
			M20 = M20*1.6;
			M21 = M21*1.6;
			M22 = M22*1.6;
			Yoffset = Round(-512.0 * 0.6);

			SE_EDGE = 2;
			SE_YOUT_QBIT = 6;
			SE_COUT_QBIT = 5;

			break;

		case MTK_CONTROL_EFFECT_MODE_BLACKBOARD: // Blackboard
			M10 = 0;
			M11 = 0;
			M12 = 0;
			M20 = 0;
			M21 = 0;
			M22 = 0;

			SE_Y = 1;
			SE_Y_CONST = 0;
			SE_EDGE = 2;
			SE_OILEN = 1;
			SE_KNEESEL = 3;
			SE_EGAIN_HB = 31;
			SE_EGAIN_HA = 15;
			SE_SPECIPONLY = 1;
			SE_SPECIGAIN = 2;
			SE_EGAIN_VB = 31;
			SE_EGAIN_VA = 15;
			SE_SPECIABS = 0;
			SE_SPECIINV = 0;
			SE_COREH = 0;
			SE_E_TH1_V = 4;
			SE_EMBOSS1 = 1;
			SE_EMBOSS2 = 1;

			break;

		case MTK_CONTROL_EFFECT_MODE_WHITEBOARD:		// Whiteboard
			M10 = 0;
			M11 = 0;
			M12 = 0;
			M20 = 0;
			M21 = 0;
			M22 = 0;

			SE_Y_CONST = 255;
			SE_Y = 1;
			SE_EDGE = 2;
			SE_OILEN = 1;
			SE_KNEESEL = 3;
			SE_EGAIN_HB = 31;
			SE_EGAIN_HA = 15;
			SE_SPECIPONLY = 1;
			SE_SPECIGAIN = 2;
			SE_EGAIN_VB = 31;
			SE_EGAIN_VA = 15;
			SE_SPECIABS = 0;
			SE_SPECIINV = 1;
			SE_COREH = 0;
			SE_E_TH1_V = 4;
			SE_EMBOSS1 = 1;
			SE_EMBOSS2 = 1;

			break;

    case MTK_CONTROL_EFFECT_MODE_NASHVILLE:
    V = Lomo_V_param[customer_vignette[LOMO_NASHVILLE-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_HEFE:
    V = Lomo_V_param[customer_vignette[LOMO_HEFE-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_VALENCIA :
    V = Lomo_V_param[customer_vignette[LOMO_VALENCIA-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_XPROII :
    V = Lomo_V_param[customer_vignette[LOMO_XPROII-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_LOFI :
    V = Lomo_V_param[customer_vignette[LOMO_LOFI-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_SIERRA :     	
    V = Lomo_V_param[customer_vignette[LOMO_SIERRA-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_KELVIN :    	
    V = Lomo_V_param[customer_vignette[LOMO_KELVIN-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_WALDEN :
    V = Lomo_V_param[customer_vignette[LOMO_WALDEN-1]+3]; 
    break;
    case MTK_CONTROL_EFFECT_MODE_F1977 :
    V = Lomo_V_param[customer_vignette[LOMO_F1977-1]+3];    	    
    break;
    case MTK_CONTROL_EFFECT_MODE_OFF: //  Do nothing.
    case MTK_CONTROL_EFFECT_MODE_SOLARIZE: //  Unsupport.
    default:
        break;

	}
	
	switch (eIdx_Effect) {
        case MTK_CONTROL_EFFECT_MODE_NASHVILLE :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_NASHVILLE\n");
        case MTK_CONTROL_EFFECT_MODE_HEFE :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_HEFE\n");        	
        case MTK_CONTROL_EFFECT_MODE_VALENCIA :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_VALENCIA\n");
        case MTK_CONTROL_EFFECT_MODE_XPROII :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_XPROII\n");
        case MTK_CONTROL_EFFECT_MODE_LOFI :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_LOFI\n");
        case MTK_CONTROL_EFFECT_MODE_SIERRA :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_SIERRA\n");
        case MTK_CONTROL_EFFECT_MODE_KELVIN :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_KELVIN\n");
        case MTK_CONTROL_EFFECT_MODE_WALDEN :
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_WALDEN\n");
        case MTK_CONTROL_EFFECT_MODE_F1977 :        //                     MY_LOG("V=%v, Cb=%f, Cr=%f, V,Cb,Cr);
        	MY_LOG("  MTK_CONTROL_EFFECT_MODE_F1977\n");
            Cb = Lomo_cb_param[customer_color_cb[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE]+2];
            Cr = Lomo_cb_param[customer_color_cr[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE]+2];
            MY_LOG("V_0=%d, Cb=%f, Cr=%f\n", V,Cb,Cr);
            V = Lomo_V_param[customer_vignette[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE]+3];    	 
            MY_LOG("V_1=%d, Cb=%f, Cr=%f\n", V,Cb,Cr);
            if (V > 1) {	// blending with 
                loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0 * (2-V) + 266 * (V-1));
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1 * (2-V) + (-342 * (V-1)));
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2 * (2-V) + 107 * (V-1));
            }
            else if (V < 1) {	// blending with no vignette effect setting
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0 * V + 255 * (1-V) + 0.5);
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1 * V +   0 * (1-V) + 0.5);
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2 * V +   0 * (1-V) + 0.5);
            }	
            else
            {
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0 * V);
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1 * V);
                 loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2 = (int)DATA_11BIT_2COMP_DATA(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2 * V);
            }	
            loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_EN = (int)(LomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_EN);

            MY_LOG("LOMO EFFECT = %d",eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1);
            MY_LOG("loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0[%d] = 0x%8x \n", eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1, loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P0);
            MY_LOG("loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1[%d] = 0x%8x \n", eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1, loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P1);
            MY_LOG("loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2[%d] = 0x%8x \n", eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1, loadLomoFilterG2C[eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1].G2C_SHADE_P2);
            MY_LOG("LOMO_PARA_GET_SHADE(%d,%d)=0x%8x  \n",LOMO_PARA_G2C_SHADE_con_1,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1,LOMO_PARA_GET_SHADE(LOMO_PARA_G2C_SHADE_con_1,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1));
            MY_LOG("LOMO_PARA_GET_SHADE(%d,%d)=0x%8x  \n",LOMO_PARA_G2C_SHADE_con_2,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1,LOMO_PARA_GET_SHADE(LOMO_PARA_G2C_SHADE_con_2,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1));
            MY_LOG("LOMO_PARA_GET_SHADE(%d,%d)=0x%8x  \n",LOMO_PARA_G2C_SHADE_con_3,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1,LOMO_PARA_GET_SHADE(LOMO_PARA_G2C_SHADE_con_3,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1));
            MY_LOG("LOMO_PARA_GET_SHADE(%d,%d)=0x%8x  \n",LOMO_PARA_G2C_SHADE_tar,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1,LOMO_PARA_GET_SHADE(LOMO_PARA_G2C_SHADE_tar,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1));            
            MY_LOG("LOMO_PARA_GET_SHADE(%d,%d)=0x%8x  \n",LOMO_PARA_G2C_SHADE_sp,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1,LOMO_PARA_GET_SHADE(LOMO_PARA_G2C_SHADE_sp,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1));
            // Write back: G2C_SHARE
            for(i=0;i<LOMO_PARA_G2C_SHADE_NUM;i++)
            {
                MY_LOG("HW rG2C_SHADE.set[%d] = 0x%8x\n",i,rG2C_SHADE.set[i]);
                rG2C_SHADE.set[i] = LOMO_PARA_GET_SHADE(i,eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1);
                MY_LOG("Lomo rG2C_SHADE.set[%d] = 0x%8x\n",i,rG2C_SHADE.set[i]);
            }


        break;    
        default:
         MY_LOG("No related to Lomo Effect");
            // Write back: G2C_SHARE
            for(i=0;i<LOMO_PARA_G2C_SHADE_NUM;i++)
            {
                MY_LOG("HW rG2C_SHADE.set[%d] = 0x%8x\n",i,rG2C_SHADE.set[i]);
                rG2C_SHADE.set[i] = LOMO_PARA_GET_SHADE(i,MTK_CONTROL_EFFECT_MODE_OFF);
                MY_LOG("NON-Lomo rG2C_SHADE.set[%d] = 0x%8x\n",i,rG2C_SHADE.set[i]);
            }         
         break;
    	}


	H = H_param[rIspUsrSelectLevel.eIdx_Hue] * PI / 180;
	S = S_param[rIspUsrSelectLevel.eIdx_Sat];
	B = B_param[rIspUsrSelectLevel.eIdx_Bright];
	C = C_param[rIspUsrSelectLevel.eIdx_Contrast];

    MY_LOG("H=%f, S=%f, B=%f, C=%f \n",H, S, B, C);

	YR = LIMIT(Round(C * M00), -1023, 1023);
	YG = LIMIT(Round(C * M01), -1023, 1023);
	YB = LIMIT(Round(C * M02), -1023, 1023);
	UR = LIMIT(Round(S * cos(H) * M10 - S * sin(H) * M20), -1023, 1023);
	UG = LIMIT(Round(S * cos(H) * M11 - S * sin(H) * M21), -1023, 1023);
	UB = LIMIT(Round(S * cos(H) * M12 - S * sin(H) * M22), -1023, 1023);
	VR = LIMIT(Round(S * sin(H) * M10 + S * cos(H) * M20), -1023, 1023);
	VG = LIMIT(Round(S * sin(H) * M11 + S * cos(H) * M21), -1023, 1023);
	VB = LIMIT(Round(S * sin(H) * M12 + S * cos(H) * M22), -1023, 1023);

	Y_OFFSET11 = LIMIT(Round(Yoffset + B - 512 * (C-1)), -1023, 1023);
	U_OFFSET10 = LIMIT(Round(Uoffset * S+Cb), -511, 511);
	V_OFFSET10 = LIMIT(Round(Voffset * S+Cr), -511, 511);

	MY_LOG("YR=%d, YG=%d, YB=%d \n",YR, YG, YB);
	MY_LOG("UR=%d, UG=%d, UB=%d \n",UR, UG, UB);
	MY_LOG("VR=%d, VG=%d, VB=%d \n",VR, VG, VB);
	MY_LOG("Y_OFFSET11=%d, U_OFFSET10=%d, V_OFFSET10=%d \n",Y_OFFSET11, U_OFFSET10, V_OFFSET10);

    // Write back: G2C
    rG2C.conv_0a.bits.G2C_CNV_00 = (YR >= 0) ? static_cast<MUINT32>(YR) : static_cast<MUINT32>(2048 + YR);
    rG2C.conv_0a.bits.G2C_CNV_01 = (YG >= 0) ? static_cast<MUINT32>(YG) : static_cast<MUINT32>(2048 + YG);
    rG2C.conv_0b.bits.G2C_CNV_02 = (YB >= 0) ? static_cast<MUINT32>(YB) : static_cast<MUINT32>(2048 + YB);
    rG2C.conv_0b.bits.G2C_Y_OFST = (Y_OFFSET11 >= 0) ? static_cast<MUINT32>(Y_OFFSET11) : static_cast<MUINT32>(2048 + Y_OFFSET11);
    rG2C.conv_1a.bits.G2C_CNV_10 = (UR >= 0) ? static_cast<MUINT32>(UR) : static_cast<MUINT32>(2048 + UR);
    rG2C.conv_1a.bits.G2C_CNV_11 = (UG >= 0) ? static_cast<MUINT32>(UG) : static_cast<MUINT32>(2048 + UG);
    rG2C.conv_1b.bits.G2C_CNV_12 = (UB >= 0) ? static_cast<MUINT32>(UB) : static_cast<MUINT32>(2048 + UB);
    rG2C.conv_1b.bits.G2C_U_OFST = (U_OFFSET10 >= 0) ? static_cast<MUINT32>(U_OFFSET10) : static_cast<MUINT32>(1024 + U_OFFSET10);
    rG2C.conv_2a.bits.G2C_CNV_20 = (VR >= 0) ? static_cast<MUINT32>(VR) : static_cast<MUINT32>(2048 + VR);
    rG2C.conv_2a.bits.G2C_CNV_21 = (VG >= 0) ? static_cast<MUINT32>(VG) : static_cast<MUINT32>(2048 + VG);
    rG2C.conv_2b.bits.G2C_CNV_22 = (VB >= 0) ? static_cast<MUINT32>(VB) : static_cast<MUINT32>(2048 + VB);
    rG2C.conv_2b.bits.G2C_V_OFST = (V_OFFSET10 >= 0) ? static_cast<MUINT32>(V_OFFSET10) : static_cast<MUINT32>(1024 + V_OFFSET10);

    MY_LOG("rG2C.conv_0a.bits.G2C_CNV_00=0x%8x \n", rG2C.conv_0a.bits.G2C_CNV_00);
    MY_LOG("rG2C.conv_0a.bits.G2C_CNV_01=0x%8x \n", rG2C.conv_0a.bits.G2C_CNV_01);
    MY_LOG("rG2C.conv_0b.bits.G2C_CNV_02=0x%8x \n", rG2C.conv_0b.bits.G2C_CNV_02);
    MY_LOG("rG2C.conv_0b.bits.G2C_Y_OFST=0x%8x \n", rG2C.conv_0b.bits.G2C_Y_OFST);
    MY_LOG("rG2C.conv_1a.bits.G2C_CNV_10=0x%8x \n", rG2C.conv_1a.bits.G2C_CNV_10);
    MY_LOG("rG2C.conv_1a.bits.G2C_CNV_11=0x%8x \n", rG2C.conv_1a.bits.G2C_CNV_11);
    MY_LOG("rG2C.conv_1b.bits.G2C_CNV_12=0x%8x \n", rG2C.conv_1b.bits.G2C_CNV_12);
    MY_LOG("rG2C.conv_1b.bits.G2C_U_OFST=0x%8x \n", rG2C.conv_1b.bits.G2C_U_OFST);
    MY_LOG("rG2C.conv_2a.bits.G2C_CNV_20=0x%8x \n", rG2C.conv_2a.bits.G2C_CNV_20);
    MY_LOG("rG2C.conv_2a.bits.G2C_CNV_21=0x%8x \n", rG2C.conv_2a.bits.G2C_CNV_21);
    MY_LOG("rG2C.conv_2b.bits.G2C_CNV_22=0x%8x \n", rG2C.conv_2b.bits.G2C_CNV_22);
    MY_LOG("rG2C.conv_2b.bits.G2C_V_OFST=0x%8x \n", rG2C.conv_2b.bits.G2C_V_OFST);

    // Write back: SE
    rSE.out_edge_ctrl.bits.SEEE_OUT_EDGE_SEL = SE_EDGE;
    rSE.y_ctrl.bits.SEEE_SE_YOUT_QBIT = SE_YOUT_QBIT;
    rSE.y_ctrl.bits.SEEE_SE_COUT_QBIT = SE_COUT_QBIT;
    rSE.y_ctrl.bits.SEEE_SE_CONST_Y_EN = SE_Y;
    rSE.y_ctrl.bits.SEEE_SE_CONST_Y_VAL = SE_Y_CONST;
    rSE.edge_ctrl_3.bits.SEEE_SE_OIL_EN = SE_OILEN;
    rSE.special_ctrl.bits.SEEE_SE_KNEE_SEL = SE_KNEESEL;
    rSE.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_B = SE_EGAIN_HB;
    rSE.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_A = SE_EGAIN_HA;
    rSE.special_ctrl.bits.SEEE_SE_SPECL_HALF_MODE = SE_SPECIPONLY;
    rSE.special_ctrl.bits.SEEE_SE_SPECL_GAIN = SE_SPECIGAIN;
    rSE.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_B = SE_EGAIN_VB;
    rSE.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_A = SE_EGAIN_VA;
    rSE.special_ctrl.bits.SEEE_SE_SPECL_ABS = SE_SPECIABS;
    rSE.special_ctrl.bits.SEEE_SE_SPECL_INV = SE_SPECIINV;
    rSE.core_ctrl_1.bits.SEEE_SE_CORE_HORI_X0 = SE_COREH;
    rSE.core_ctrl_2.bits.SEEE_SE_CORE_VERT_X0 = SE_E_TH1_V;
    rSE.edge_ctrl_2.bits.SEEE_SE_BOSS_IN_SEL = SE_EMBOSS1;
    rSE.edge_ctrl_2.bits.SEEE_SE_BOSS_GAIN_OFF = SE_EMBOSS2;

    MY_LOG("rSE.out_edge_ctrl.bits.SEEE_OUT_EDGE_SEL=%d \n",rSE.out_edge_ctrl.bits.SEEE_OUT_EDGE_SEL);
	MY_LOG("rSE.y_ctrl.bits.SEEE_SE_YOUT_QBIT=%d \n",rSE.y_ctrl.bits.SEEE_SE_YOUT_QBIT);
	MY_LOG("rSE.y_ctrl.bits.SEEE_SE_COUT_QBIT=%d \n",rSE.y_ctrl.bits.SEEE_SE_COUT_QBIT);        
	MY_LOG("rSE.y_ctrl.bits.SEEE_SE_CONST_Y_EN=%d \n",rSE.y_ctrl.bits.SEEE_SE_CONST_Y_EN);
	MY_LOG("rSE.y_ctrl.bits.SEEE_SE_CONST_Y_VAL=%d \n",rSE.y_ctrl.bits.SEEE_SE_CONST_Y_VAL);
	MY_LOG("rSE.edge_ctrl_3.bits.SEEE_SE_OIL_EN=%d \n",rSE.edge_ctrl_3.bits.SEEE_SE_OIL_EN);
	MY_LOG("rSE.special_ctrl.bits.SEEE_SE_KNEE_SEL=%d \n",rSE.special_ctrl.bits.SEEE_SE_KNEE_SEL);
	MY_LOG("rSE.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_B=%d \n",rSE.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_B);
	MY_LOG("rSE.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_A=%d \n",rSE.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_A);
	MY_LOG("rSE.special_ctrl.bits.SEEE_SE_SPECL_HALF_MODE=%d \n",rSE.special_ctrl.bits.SEEE_SE_SPECL_HALF_MODE);
	MY_LOG("rSE.special_ctrl.bits.SEEE_SE_SPECL_GAIN=%d \n",rSE.special_ctrl.bits.SEEE_SE_SPECL_GAIN);
	MY_LOG("rSE.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_B=%d \n",rSE.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_B);
	MY_LOG("rSE.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_A=%d \n",rSE.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_A);
	MY_LOG("rSE.special_ctrl.bits.SEEE_SE_SPECL_ABS=%d \n",rSE.special_ctrl.bits.SEEE_SE_SPECL_ABS);
	MY_LOG("rSE.special_ctrl.bits.SEEE_SE_SPECL_INV=%d \n",rSE.special_ctrl.bits.SEEE_SE_SPECL_INV);
	MY_LOG("rSE.core_ctrl_1.bits.SEEE_SE_CORE_HORI_X0=%d \n",rSE.core_ctrl_1.bits.SEEE_SE_CORE_HORI_X0);
	MY_LOG("rSE.core_ctrl_2.bits.SEEE_SE_CORE_VERT_X0=%d \n",rSE.core_ctrl_2.bits.SEEE_SE_CORE_VERT_X0);
	MY_LOG("rSE.edge_ctrl_2.bits.SEEE_SE_BOSS_IN_SEL=%d \n",rSE.edge_ctrl_2.bits.SEEE_SE_BOSS_IN_SEL);
	MY_LOG("rSE.edge_ctrl_2.bits.SEEE_SE_BOSS_GAIN_OFF=%d \n",rSE.edge_ctrl_2.bits.SEEE_SE_BOSS_GAIN_OFF);

    MY_LOG("rSE.out_edge_ctrl.val=0x%8x \n",rSE.out_edge_ctrl.val);
    MY_LOG("rSE.y_ctrl.val=0x%8x \n",rSE.y_ctrl.val);
    MY_LOG("rSE.edge_ctrl_1.val=0x%8x \n",rSE.edge_ctrl_1.val);
    MY_LOG("rSE.edge_ctrl_2.val=0x%8x \n",rSE.edge_ctrl_2.val);
    MY_LOG("rSE.edge_ctrl_3.val=0x%8x \n",rSE.edge_ctrl_3.val);
    MY_LOG("rSE.special_ctrl.val=0x%8x \n",rSE.special_ctrl.val);
    MY_LOG("rSE.core_ctrl_1.val=0x%8x \n",rSE.core_ctrl_1.val);
    MY_LOG("rSE.core_ctrl_2.val=0x%8x \n",rSE.core_ctrl_2.val);

}

MVOID
IspTuningCustom::userSetting_EFFECT_GGM(
        RAWIspCamInfo const& rCamInfo, EIndex_Effect_T const& eIdx_Effect,  ISP_NVRAM_GGM_T& rGGM
    )
{
    MUINT32 idx;
    switch (eIdx_Effect) {
        case MTK_CONTROL_EFFECT_MODE_NASHVILLE :
        case MTK_CONTROL_EFFECT_MODE_HEFE :
        case MTK_CONTROL_EFFECT_MODE_VALENCIA :
        case MTK_CONTROL_EFFECT_MODE_XPROII :
        case MTK_CONTROL_EFFECT_MODE_LOFI :
        case MTK_CONTROL_EFFECT_MODE_SIERRA :
        case MTK_CONTROL_EFFECT_MODE_KELVIN :
        case MTK_CONTROL_EFFECT_MODE_WALDEN :
        case MTK_CONTROL_EFFECT_MODE_F1977 :      
            for(idx=0; idx<CUSTOM_LOMO_GGM_GAIN_NUM;idx++)
            {        
//                rGGM.lut_rb.set[idx]=LomoFilterGGM[0][CUSTOM_LOMO_GGM_CHANNEL_BR][idx];
//                rGGM.lut_g.set[idx]=LomoFilterGGM[0][CUSTOM_LOMO_GGM_CHANNEL_G][idx];
                rGGM.lut_rb.set[idx]=LomoFilterGGM[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_BR][idx];
                rGGM.lut_g.set[idx]=LomoFilterGGM[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_G][idx];

            }
            MY_LOG("LOMO EFFECT = %d",eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1);
            MY_LOG("rGGM.lut_rb.set[0] = 0x%8x", rGGM.lut_rb.set[0]);
            MY_LOG("rGGM.lut_rb.set[1] = 0x%8x", rGGM.lut_rb.set[1]);
            MY_LOG("rGGM.lut_g.set[0] = 0x%8x", rGGM.lut_g.set[0]);
            MY_LOG("rGGM.lut_g.set[1] = 0x%8x", rGGM.lut_g.set[1]);        
        break;    
        default:
            MY_LOG("No need to change GGM from NVRAM");	
            MY_LOG("rGGM.lut_rb.set[0] = 0x%8x", rGGM.lut_rb.set[0]);
            MY_LOG("rGGM.lut_rb.set[1] = 0x%8x", rGGM.lut_rb.set[1]);
            MY_LOG("rGGM.lut_g.set[0] = 0x%8x", rGGM.lut_g.set[0]);
            MY_LOG("rGGM.lut_g.set[1] = 0x%8x", rGGM.lut_g.set[1]);                       	
         break;
    	}
    return;
         
  }


MVOID
IspTuningCustom::userSetting_EFFECT_GGM_JNI(
        RAWIspCamInfo const& rCamInfo, EIndex_Effect_T const& eIdx_Effect,  ISP_NVRAM_GGM_T& rGGM
    )
{
    MUINT32 idx;
    switch (eIdx_Effect) {
        case MTK_CONTROL_EFFECT_MODE_NASHVILLE :
        case MTK_CONTROL_EFFECT_MODE_HEFE :
        case MTK_CONTROL_EFFECT_MODE_VALENCIA :
        case MTK_CONTROL_EFFECT_MODE_XPROII :
        case MTK_CONTROL_EFFECT_MODE_LOFI :
        case MTK_CONTROL_EFFECT_MODE_SIERRA :
        case MTK_CONTROL_EFFECT_MODE_KELVIN :
        case MTK_CONTROL_EFFECT_MODE_WALDEN :
        case MTK_CONTROL_EFFECT_MODE_F1977 :      
            for(idx=0; idx<CUSTOM_LOMO_GGM_GAIN_NUM;idx++)
            {        
                rGGM.lut_rb.set[idx]=LomoFilterGGMJni[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_BR][idx];
                rGGM.lut_g.set[idx]=LomoFilterGGMJni[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_G][idx];
            }
            MY_LOG("LOMO EFFECT = %d",eIdx_Effect-MTK_CONTROL_EFFECT_MODE_NASHVILLE+1);
            MY_LOG("rGGM.lut_rb.set[0] = 0x%8x", rGGM.lut_rb.set[0]);
            MY_LOG("rGGM.lut_rb.set[1] = 0x%8x", rGGM.lut_rb.set[1]);
            MY_LOG("rGGM.lut_g.set[0] = 0x%8x", rGGM.lut_g.set[0]);
            MY_LOG("rGGM.lut_g.set[1] = 0x%8x", rGGM.lut_g.set[1]);        
        break;    
        default:
            MY_LOG("No need to change GGM from NVRAM");	
            MY_LOG("rGGM.lut_rb.set[0] = 0x%8x", rGGM.lut_rb.set[0]);
            MY_LOG("rGGM.lut_rb.set[1] = 0x%8x", rGGM.lut_rb.set[1]);
            MY_LOG("rGGM.lut_g.set[0] = 0x%8x", rGGM.lut_g.set[0]);
            MY_LOG("rGGM.lut_g.set[1] = 0x%8x", rGGM.lut_g.set[1]);                       	
         break;
    	}
    return;
         
  }



};  //NSIspTuning


