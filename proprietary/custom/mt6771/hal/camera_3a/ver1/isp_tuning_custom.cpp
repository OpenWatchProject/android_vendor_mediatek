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

#define LOG_TAG "isp_tuning_custom"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <aaa_types.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_tuning_custom_swnr.h>
#include <isp_tuning_custom_instance.h>
#include <stdlib.h>                     // For atoi()
#include <stdio.h>
#include <cutils/properties.h>  // For property_get().
#include "camera_custom_3dnr.h"
#include "n3d_sync2a_tuning_param.h"
#include <math.h>

#define CEIL(a)   ( (a-(int)a) == 0 ? (int)a : (int)(a+1))

#define FLOOR(a)  ( (int)a)

#ifndef LOG2
#define LOG2(x)       ((MDOUBLE)log(x)/log(2))
#endif


using namespace NSIspTuning;


// ais
/*
 *  disable sl2a & sl2b. enable slb2 for ais stage3/4, disable for other cases
 *  make sure shading_tuning_custom.cpp & isp_tuning_custom.cpp use the same value
 */
#define TUNING_FOR_AIS  0
#define AIS_INTERPOLATION   0

//static float AIS_NORMAL_CFA_RATIO[4] = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively
//static float AIS_NORMAL_YNR_RATIO[4] = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively
//static float AIS_NORMAL_CNR_RATIO[4] = {0.20f, 1.00f, 0.20f, 0.20f}; //0=ais, 1=normal, for stage1-4 respectively
//static float AIS_NORMAL_EE_RATIO[4]  = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively

#define IS_AIS          (rCamInfo.rAEInfo.u4OrgRealISOValue != rCamInfo.rAEInfo.u4RealISOValue)

#define FINE_TUNE_SUPPORT_NUM 2

static ISP_FEATURE_TOP_CTL ispTopCtl[EIspProfile_NUM] =
{
    //0:disable, 1:enable
    //EIsProfile_Preview 00
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Video 01
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Capture 02
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_MFNR_Before_Blend 03
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(0)   |F_NDG2_EN(0)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(0)  |F_EE_EN(0)    |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_MFNR_Single 04
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_ADBS_EN(0)  |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(0)   |F_NDG2_EN(0)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(0)  |F_EE_EN(0)    |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_MFNR_MFB 05
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_ADBS_EN(0)  |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(0)  |F_ANR_EN(0)
    |F_SL2C_EN(0)   |F_ANR2_EN(0)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(0)   |F_NDG2_EN(0)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(0)  |F_EE_EN(0)    |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_MFNR_After_Blend 06
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_ADBS_EN(0)  |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(1)),

    //EIsProfile_Capture_MultiPass_HWNR 07
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_ADBS_EN(0)  |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)    |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_N3D_Preview 08
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(1)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(1)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_N3D_Preview_toW 09
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(1)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(1)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_N3D_Video 10
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(1)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(1)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_N3D_Video_toW 11
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(1)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(1)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_N3D_Capture 12
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_N3D_Capture_toW 13
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_N3D_Capture_Depth 14
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIspProfile_N3D_Capture_Depth_toW 15
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_iHDR_Preview 16
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Preview 17
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_mHDR_Preview 18
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_iHDR_Video 19
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Video 20
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_mHDR_Video 21
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_iHDR_Preview_VSS 22
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Preview_VSS 23
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),
    //10
    //EIsProfile_mHDR_Preview_VSS 24
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_iHDR_Video_VSS 25
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Video_VSS 26
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_mHDR_Video_VSS 27
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Capture 28
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_mHDR_Capture 29
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Capture_MFNR_Before_Blend 30
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(0)   |F_NDG2_EN(0)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(0)  |F_EE_EN(0)    |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Capture_MFNR_Single 31
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_ADBS_EN(0)  |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(0)   |F_NDG2_EN(0)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(0)  |F_EE_EN(0)    |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Capture_MFNR_MFB 32
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_ADBS_EN(0)  |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(0)  |F_ANR_EN(0)
    |F_SL2C_EN(0)   |F_ANR2_EN(0)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(0)   |F_NDG2_EN(0)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(0)  |F_EE_EN(0)    |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0)),

    //EIsProfile_zHDR_Capture_MFNR_After_Blend 33
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_ADBS_EN(0)  |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(1)),

    //EIsProfile_Auto_iHDR_Preview 34
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_zHDR_Preview 35
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_mHDR_Preview 36
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_iHDR_Video 37
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),
    //20
    //EIsProfile_Auto_zHDR_Video 38
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_mHDR_Video  39
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_iHDR_Preview_VSS 40
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_zHDR_Preview_VSS 41
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_mHDR_Preview_VSS 42
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_iHDR_Video_VSS 43
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_zHDR_Video_VSS 44
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_mHDR_Video_VSS 45
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_zHDR_Capture 46
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_Auto_mHDR_Capture 47
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_SE_Before_Blend 48
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    // EIspProfile_SWHDR_R_SE_Single 49
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    // EIspProfile_SWHDR_R_SE_MFB 50
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_SE_After_Blend  51
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(1)),

    //EIspProfile_SWHDR_R_SE_Capture  52
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_VLE_Before_Blend  53
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_VLE_Single  54
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_VLE_MFB  55
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_VLE_After_Blend  56
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(1)),

    //EIspProfile_SWHDR_R_VLE_Capture  57
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_RLE_Capture  58
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_R_SLE_MFB  59
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_V_SE_Before_Blend  60
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_V_SE_Single  61
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_V_SE_MFB  62
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_V_SE_After_Blend  63
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(1)),

    //EIspProfile_SWHDR_V_VLE_Before_Blend  64
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_V_VLE_Single  65
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_V_VLE_MFB  66
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),
    //110
    //EIspProfile_SWHDR_V_VLE_After_Blend  67
    ( F_SL2FG_EN(0)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(0)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(1)  |F_EE_EN(0)    |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(1)),

    //EIspProfile_SWHDR_Phase1 68
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_SWHDR_Phase1 69
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(1)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_Preview 70
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_Video 71
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),
    //40
    //EIsProfile_EIS_iHDR_Preview 72
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_zHDR_Preview 73
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_mHDR_Preview 74
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_iHDR_Video 75
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_zHDR_Video 76
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_mHDR_Video 77
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_Auto_iHDR_Preview 78
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_Auto_zHDR_Preview 79
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_Auto_mHDR_Preview 80
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_Auto_iHDR_Video 81
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),
    //50
    //EIsProfile_EIS_Auto_zHDR_Video 82
    ( F_SL2FG_EN(1)  |F_DBS_EN(1)   |F_ADBS_EN(0)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(1)    |F_RMG_EN(1)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_EIS_Auto_mHDR_Video 83
    ( F_SL2FG_EN(1)  |F_DBS_EN(0)   |F_ADBS_EN(1)  |F_OBC_EN(1)   |F_BPC_EN(1)   |F_NR1_EN(1)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(1)
    |F_SL2H_EN(1)   |F_RNR_EN(1)    |F_SL2_EN(1)   |F_UDM_EN(1)   |F_CCM_EN(1)   |F_CCM2_EN(0)
    |F_LCE_EN(1)    |F_GGM_EN(1)    |F_GGM2_EN(0)  |F_G2C_EN(1)   |F_SL2B_EN(1)  |F_ANR_EN(1)
    |F_SL2C_EN(1)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(1)   |F_HFG_EN(1)   |F_SL2D_EN(1)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(1)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIsProfile_YUV_Reprocess 84
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(0)  |F_ANR_EN(1)
    |F_SL2C_EN(0)   |F_ANR2_EN(1)   |F_CCR_EN(1)   |F_BOK_EN(0)   |F_NDG_EN(1)   |F_NDG2_EN(1)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(1)   |F_SL2D_EN(0)  |F_EE_EN(1)     |F_ABF_EN(1)
    |F_SL2E_EN(0)   |F_COLOR_EN(1)  |F_MIXER3_EN(0)),

    //EIspProfile_Bokeh 85
    ( F_SL2FG_EN(0)  |F_DBS_EN(0)   |F_OBC_EN(0)   |F_BPC_EN(0)   |F_NR1_EN(0)
    |F_RMM_EN(0)    |F_RMG_EN(0)    |F_RPGN_EN(0)
    |F_SL2H_EN(0)   |F_RNR_EN(0)    |F_SL2_EN(0)   |F_UDM_EN(0)   |F_CCM_EN(0)   |F_CCM2_EN(0)
    |F_LCE_EN(0)    |F_GGM_EN(0)    |F_GGM2_EN(0)  |F_G2C_EN(0)   |F_SL2B_EN(0)  |F_ANR_EN(0)
    |F_SL2C_EN(0)   |F_ANR2_EN(0)   |F_CCR_EN(0)   |F_BOK_EN(1)   |F_NDG_EN(0)   |F_NDG2_EN(0)
    |F_PCA_EN(0)    |F_SL2I_EN(0)   |F_HFG_EN(0)   |F_SL2D_EN(0)  |F_EE_EN(0)     |F_ABF_EN(0)
    |F_SL2E_EN(0)   |F_COLOR_EN(0)  |F_MIXER3_EN(0))

};

//static ISP_NVRAM_ANR_T customANR_main[FINE_TUNE_SUPPORT_NUM][NVRAM_ISP_REGS_ISO_SUPPORT_NUM] =
//{};

//static ISP_NVRAM_ANR2_T customANR2_main[FINE_TUNE_SUPPORT_NUM][NVRAM_ISP_REGS_ISO_SUPPORT_NUM] =
//{};

//static ISP_NVRAM_EE_T customEE_main[FINE_TUNE_SUPPORT_NUM][NVRAM_ISP_REGS_ISO_SUPPORT_NUM] =
//{};

static MUINT8 RawHDR_LCEGain_ConverParam[7] = {4, 40, 12, 60, 0, 8, 80};
                          //{ LvX1, LvY1, LvX2, LvY2, DrX1, DrX2, DrY2}
MUINT32 WEIGHTING(MUINT32 x, MUINT32 y, float w) {
    MUINT32 z = (((x)*(w))+((y)*(1.0f-(w))));
    return z;
}

MINT32 AIS_Profile2Stage(MUINT32 profile) {
    MINT32 stage = -1;
    switch(profile) {
        case EIspProfile_MFNR_Before_Blend:
            stage = 1;
            break;

        case EIspProfile_MFNR_Single:
            stage = 2;
            break;

        case EIspProfile_MFNR_MFB:
            stage = 3;
            break;

        case EIspProfile_MFNR_After_Blend:
            stage = 4;
            break;

        default:
            stage = -1;
    }

    return stage;
}


MUINT32 get_normal_SWNR_ENC_enable_ISO_threshold()
{
#define NORMAL_SWNR_ENC_ENABLE_ISO_THRESHOLD (800) // enable if ISO >= THRESHOLD
    return NORMAL_SWNR_ENC_ENABLE_ISO_THRESHOLD;
}

MUINT32 get_MFB_SWNR_ENC_enable_ISO_threshold()
{
#define MFB_SWNR_ENC_ENABLE_ISO_THRESHOLD (800) // enable if ISO >= THRESHOLD
    return MFB_SWNR_ENC_ENABLE_ISO_THRESHOLD;
}

/*******************************************************************************
*
*   rCamInfo
*       [in]    ISP Camera Info for RAW sensor. Its members are as below:
*
*           eIspProfile:
*               EIspProfile_Preview = 0,          // Preview
*               EIspProfile_Video,                // Video
*               EIspProfile_Capture,              // Capture
*               EIspProfile_ZSD_Capture,          // ZSD Capture
*               EIspProfile_VSS_Capture,          // VSS Capture
*               EIspProfile_PureRAW_Capture,      // Pure RAW Capture
*               EIspProfile_N3D_Preview,          // N3D Preview
*               EIspProfile_N3D_Video,            // N3D Video
*               EIspProfile_N3D_Capture,          // N3D Capture
*               EIspProfile_MFB_Capture_EE_Off,   // MFB capture: EE off
*               EIspProfile_MFB_Blending_All_Off, // MFB blending: all off
*               EIspProfile_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
*               EIspProfile_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
*               EIspProfile_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
*               EIspProfile_VFB_PostProc,         // VFB post process: all off + ANR + CCR + PCA
*               EIspProfile_IHDR_Preview,         // IHDR preview
*               EIspProfile_IHDR_Video,           // IHDR video
*
*           eSensorMode:
*               ESensorMode_Preview = 0,
*               ESensorMode_Capture,
*               ESensorMode_Video,
*               ESensorMode_SlimVideo1,
*               ESensorMode_SlimVideo2,
*
*           eIdx_Scene:
*               MTK_CONTROL_SCENE_MODE_UNSUPPORTED = 0,
*               MTK_CONTROL_SCENE_MODE_FACE_PRIORITY,
*               MTK_CONTROL_SCENE_MODE_ACTION,
*               MTK_CONTROL_SCENE_MODE_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_LANDSCAPE,
*               MTK_CONTROL_SCENE_MODE_NIGHT,
*               MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_THEATRE,
*               MTK_CONTROL_SCENE_MODE_BEACH,
*               MTK_CONTROL_SCENE_MODE_SNOW,
*               MTK_CONTROL_SCENE_MODE_SUNSET,
*               MTK_CONTROL_SCENE_MODE_STEADYPHOTO,
*               MTK_CONTROL_SCENE_MODE_FIREWORKS,
*               MTK_CONTROL_SCENE_MODE_SPORTS,
*               MTK_CONTROL_SCENE_MODE_PARTY,
*               MTK_CONTROL_SCENE_MODE_CANDLELIGHT,
*               MTK_CONTROL_SCENE_MODE_BARCODE,
*               MTK_CONTROL_SCENE_MODE_NORMAL,
*               MTK_CONTROL_SCENE_MODE_HDR,
*
*           u4ISOValue:
*               ISO value to determine eISO.
*
*           eIdx_ISO:
*               eIDX_ISO_100 = 0,
*               eIDX_ISO_200,
*               eIDX_ISO_400,
*               eIDX_ISO_800,
*               eIDX_ISO_1200,
*               eIDX_ISO_1600,
*               eIDX_ISO_2000,
*               eIDX_ISO_2400,
*               eIDX_ISO_2800,
*               eIDX_ISO_3200,
*
*           eIdx_PCA_LUT:
*               eIDX_PCA_LOW  = 0,
*               eIDX_PCA_MIDDLE,
*               eIDX_PCA_HIGH,
*               eIDX_PCA_LOW_2,    // for video HDR only
*               eIDX_PCA_MIDDLE_2, // for video HDR only
*               eIDX_PCA_HIGH_2    // for video HDR only
*
*           eIdx_CCM:
*               eIDX_CCM_D65  = 0,
*               eIDX_CCM_TL84,
*               eIDX_CCM_CWF,
*               eIDX_CCM_A,
*
*           eIdx_Shading_CCT:
*               eIDX_Shading_CCT_BEGIN = 0,
*               eIDX_Shading_CCT_ALight = eIDX_Shading_CCT_BEGIN,
*               eIDX_Shading_CCT_CWF,
*               eIDX_Shading_CCT_D65,
*               eIDX_Shading_CCT_RSVD
*
*           rAWBInfo:
*               rProb; // Light source probability
*               rLightStat; // Light source statistics
*               rLightAWBGain; // Golden sample's AWB gain for multi-CCM
*               rCurrentAWBGain; // Current preview AWB gain
*               i4NeutralParentBlkNum; // Neutral parent block number
*               i4CCT; // CCT
*               i4FluorescentIndex; // Fluorescent index
*               i4DaylightFluorescentIndex; // Daylight fluorescent index
*               i4SceneLV; // Scene LV
*               i4AWBMode; // AWB mode
*               bAWBStable; // AWB stable
*
*           rAEInfo:
*               u4AETarget;
*               u4AECurrentTarget;
*               u4Eposuretime;
*               u4AfeGain;
*               u4IspGain;
*               u4RealISOValue;
*               i4LightValue_x10;
*               u4AECondition;
*               eAEMeterMode;
*               i2FlareOffset;
*               u2Histogrm[AE_HISTOGRAM_BIN];
*           rAFInfo:
*               i4AFPos
*
*           rFlashInfo:
*               flashMode;
*               isFlash; // 0: no flash, 1: image with flash
*
*           u4ZoomRatio_x100:
*               zoom ratio (x100)
*
*           i4LightValue_x10:
*               light value (x10)
*
*******************************************************************************/
MVOID
IspTuningCustom::
refine_CamInfo(RAWIspCamInfo& rCamInfo)
{
    (void)rCamInfo;
}

/*******************************************************************************
*
*   rCamInfo
*       [in]    ISP Camera Info for RAW sensor. Its members are as below:
*
*           eIspProfile:
*               EIspProfile_Preview = 0,          // Preview
*               EIspProfile_Video,                // Video
*               EIspProfile_Capture,              // Capture
*               EIspProfile_ZSD_Capture,          // ZSD Capture
*               EIspProfile_VSS_Capture,          // VSS Capture
*               EIspProfile_PureRAW_Capture,      // Pure RAW Capture
*               EIspProfile_N3D_Preview,          // N3D Preview
*               EIspProfile_N3D_Video,            // N3D Video
*               EIspProfile_N3D_Capture,          // N3D Capture
*               EIspProfile_MFB_Capture_EE_Off,   // MFB capture: EE off
*               EIspProfile_MFB_Blending_All_Off, // MFB blending: all off
*               EIspProfile_MFB_PostProc_EE_Off,  // MFB post process: capture + EE off
*               EIspProfile_MFB_PostProc_ANR_EE,  // MFB post process: capture + ANR + EE
*               EIspProfile_MFB_PostProc_Mixing,  // MFB post process: mixing + all off
*               EIspProfile_VFB_PostProc,         // VFB post process: all off + ANR + CCR + PCA
*               EIspProfile_IHDR_Preview,         // IHDR preview
*               EIspProfile_IHDR_Video,           // IHDR video
*
*           eSensorMode:
*               ESensorMode_Preview = 0,
*               ESensorMode_Capture,
*               ESensorMode_Video,
*               ESensorMode_SlimVideo1,
*               ESensorMode_SlimVideo2,
*
*           eIdx_Scene:
*               MTK_CONTROL_SCENE_MODE_UNSUPPORTED = 0,
*               MTK_CONTROL_SCENE_MODE_FACE_PRIORITY,
*               MTK_CONTROL_SCENE_MODE_ACTION,
*               MTK_CONTROL_SCENE_MODE_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_LANDSCAPE,
*               MTK_CONTROL_SCENE_MODE_NIGHT,
*               MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT,
*               MTK_CONTROL_SCENE_MODE_THEATRE,
*               MTK_CONTROL_SCENE_MODE_BEACH,
*               MTK_CONTROL_SCENE_MODE_SNOW,
*               MTK_CONTROL_SCENE_MODE_SUNSET,
*               MTK_CONTROL_SCENE_MODE_STEADYPHOTO,
*               MTK_CONTROL_SCENE_MODE_FIREWORKS,
*               MTK_CONTROL_SCENE_MODE_SPORTS,
*               MTK_CONTROL_SCENE_MODE_PARTY,
*               MTK_CONTROL_SCENE_MODE_CANDLELIGHT,
*               MTK_CONTROL_SCENE_MODE_BARCODE,
*               MTK_CONTROL_SCENE_MODE_NORMAL,
*               MTK_CONTROL_SCENE_MODE_HDR,
*
*           u4ISOValue:
*               ISO value to determine eISO.
*
*           eIdx_ISO:
*               eIDX_ISO_100 = 0,
*               eIDX_ISO_200,
*               eIDX_ISO_400,
*               eIDX_ISO_800,
*               eIDX_ISO_1200,
*               eIDX_ISO_1600,
*               eIDX_ISO_2000,
*               eIDX_ISO_2400,
*               eIDX_ISO_2800,
*               eIDX_ISO_3200,
*
*           eIdx_PCA_LUT:
*               eIDX_PCA_LOW  = 0,
*               eIDX_PCA_MIDDLE,
*               eIDX_PCA_HIGH,
*               eIDX_PCA_LOW_2,    // for video HDR only
*               eIDX_PCA_MIDDLE_2, // for video HDR only
*               eIDX_PCA_HIGH_2    // for video HDR only
*
*           eIdx_CCM:
*               eIDX_CCM_D65  = 0,
*               eIDX_CCM_TL84,
*               eIDX_CCM_CWF,
*               eIDX_CCM_A,
*
*           eIdx_Shading_CCT:
*               eIDX_Shading_CCT_BEGIN = 0,
*               eIDX_Shading_CCT_ALight = eIDX_Shading_CCT_BEGIN,
*               eIDX_Shading_CCT_CWF,
*               eIDX_Shading_CCT_D65,
*               eIDX_Shading_CCT_RSVD
*
*           rAWBInfo:
*               rProb; // Light source probability
*               rLightStat; // Light source statistics
*               rLightAWBGain; // Golden sample's AWB gain for multi-CCM
*               rCurrentAWBGain; // Current preview AWB gain
*               i4NeutralParentBlkNum; // Neutral parent block number
*               i4CCT; // CCT
*               i4FluorescentIndex; // Fluorescent index
*               i4DaylightFluorescentIndex; // Daylight fluorescent index
*               i4SceneLV; // Scene LV
*               i4AWBMode; // AWB mode
*               bAWBStable; // AWB stable
*
*           rAEInfo:
*               u4AETarget;
*               u4AECurrentTarget;
*               u4Eposuretime;
*               u4AfeGain;
*               u4IspGain;
*               u4RealISOValue;
*               i4LightValue_x10;
*               u4AECondition;
*               eAEMeterMode;
*               i2FlareOffset;
*               u2Histogrm[AE_HISTOGRAM_BIN];
*           rAFInfo:
*               i4AFPos
*
*           rFlashInfo:
*               flashMode;
*               isFlash; // 0: no flash, 1: image with flash
*
*           u4ZoomRatio_x100:
*               zoom ratio (x100)
*
*           i4LightValue_x10:
*               light value (x10)
*
*   rIdxMgr:
*       [in]    The default ISP tuning index manager.
*       [out]   The ISP tuning index manager after customizing.
*
*
*******************************************************************************/

MINT32 GetDBSGNP(MINT32 Ratio)
{
     if(Ratio > 11)
         return 4;
    else if (Ratio > 5)
         return 3;
    else if(Ratio > 2)
         return 2;
    else if(Ratio > 1)
         return 1;
    else
         return 0;


}

MVOID
IspTuningCustom::
refine_SL2F(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_SL2_T& rSL2F)
{
(void)rSL2F;
(void)rIspRegMgr;
(void)rCamInfo;
}

MVOID
IspTuningCustom::
refine_DBS(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_DBS_T& rDBS)
{
    (void)rIspRegMgr;
        MINT32 debugDump = property_get_int32("vendor.debug.dbs.dump", 0);
        //> prepar register value

        MINT32 aeLeSeRatio = rCamInfo.rAEInfo.i4LESE_Ratio;
        MINT32 aeLeSeRatio_round = (rCamInfo.rAEInfo.i4LESE_Ratio + 50) / 100;

        // Use Formula to calculate
        if(aeLeSeRatio < 100)
        {
            aeLeSeRatio = 100;
            aeLeSeRatio_round = 1;
        }
        if(aeLeSeRatio > 1600)
        {
            aeLeSeRatio = 1600;
            aeLeSeRatio_round = 16;
        }
        //MINT32 DBS_GN  = (aeLeSeRatio << 3) / 100;
        MINT32 DBS_GN  = ((aeLeSeRatio << 3) + 50) / 100; // do rounding
        //MINT32 DBS_GN2 = 6400 / aeLeSeRatio;
        MINT32 DBS_GN2 = (12800 / aeLeSeRatio + 1) >> 1; // do rounding
        MINT32 DBS_GNP = GetDBSGNP(aeLeSeRatio_round);

    MY_LOG_IF(debugDump, "refine_dbs, i4LESE_Ratio(%d)->asLeSeRatio_round(%d),GNP(%d), GAIN1(%d),GAIN2(%d)", aeLeSeRatio, aeLeSeRatio_round, DBS_GNP, DBS_GN, DBS_GN2);

        rDBS.ctl.val = (rDBS.ctl.val & 0xFFF8)
                                    | (DBS_GNP&0x07)
                                    | ((DBS_GN&0xFF) << 16)
                                    | ((DBS_GN2&0xFF) << 24);
    }




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



MVOID
IspTuningCustom::
refine_OBC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_OBC_T& rOBC)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rOBC;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    if (getSensorDev() == ESensorDev_Main) { // main
    }
    else if (getSensorDev() == ESensorDev_Sub) { // sub
    }
    else { // main2
    }

    MY_LOG("rOBC.offst0 = 0x%8x", rOBC.offst0);
    MY_LOG("rOBC.offst1 = 0x%8x", rOBC.offst1);
    MY_LOG("rOBC.offst2 = 0x%8x", rOBC.offst2);
    MY_LOG("rOBC.offst3 = 0x%8x", rOBC.offst3);
    MY_LOG("rOBC.gain0 = 0x%8x", rOBC.gain0);
    MY_LOG("rOBC.gain1 = 0x%8x", rOBC.gain1);
    MY_LOG("rOBC.gain2 = 0x%8x", rOBC.gain2);
    MY_LOG("rOBC.gain3 = 0x%8x", rOBC.gain3);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_BPC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_BPC_T& rBPC)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rBPC;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rBPC.con = 0x%8x", rBPC.con);
    MY_LOG("rBPC.th1 = 0x%8x", rBPC.th1);
    MY_LOG("rBPC.th2 = 0x%8x", rBPC.th2);
    MY_LOG("rBPC.th3 = 0x%8x", rBPC.th3);
    MY_LOG("rBPC.th4 = 0x%8x", rBPC.th4);
    MY_LOG("rBPC.dtc = 0x%8x", rBPC.dtc);
    MY_LOG("rBPC.cor = 0x%8x", rBPC.cor);
    MY_LOG("rBPC.tbli1 = 0x%8x", rBPC.tbli1);
    MY_LOG("rBPC.th1_c = 0x%8x", rBPC.th1_c);
    MY_LOG("rBPC.th2_c = 0x%8x", rBPC.th2_c);
    MY_LOG("rBPC.th3_c = 0x%8x", rBPC.th3_c);
    #endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_NR1(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_NR1_T& rNR1)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rNR1;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rNR1.con = 0x%8x", rNR1.con);
    MY_LOG("rNR1.ct_con = 0x%8x", rNR1.ct_con);
    #endif
}

MVOID
IspTuningCustom::
refine_PDC(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_BNR_PDC_T& rPDC)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rPDC;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rNR1.con = 0x%8x", rNR1.con);
    MY_LOG("rNR1.ct_con = 0x%8x", rNR1.ct_con);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_SL2(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_SL2_T& rSL2)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rSL2;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rSL2.cen = 0x%8x", rSL2.cen);
    MY_LOG("rSL2.max0_rr = 0x%8x", rSL2.max0_rr);
    MY_LOG("rSL2.max1_rr = 0x%8x", rSL2.max1_rr);
    MY_LOG("rSL2.max2_rr = 0x%8x", rSL2.max2_rr);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_RPG(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_RPG_T& rRPG)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rRPG;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rRPG.satu_1 = 0x%8x", rRPG.satu_1);
    MY_LOG("rRPG.satu_2 = 0x%8x", rRPG.satu_2);
    MY_LOG("rRPG.gain_1 = 0x%8x", rRPG.gain_1);
    MY_LOG("rRPG.gain_2 = 0x%8x", rRPG.gain_2);
    MY_LOG("rRPG.ofst_1 = 0x%8x", rRPG.ofst_1);
    MY_LOG("rRPG.ofst_2 = 0x%8x", rRPG.ofst_2);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_PGN(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_PGN_T& rPGN)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rPGN;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rPGN.satu_1 = 0x%8x", rPGN.satu_1);
    MY_LOG("rPGN.satu_2 = 0x%8x", rPGN.satu_2);
    MY_LOG("rPGN.gain_1 = 0x%8x", rPGN.gain_1);
    MY_LOG("rPGN.gain_2 = 0x%8x", rPGN.gain_2);
    MY_LOG("rPGN.ofst_1 = 0x%8x", rPGN.ofst_1);
    MY_LOG("rPGN.ofst_2 = 0x%8x", rPGN.ofst_2);
    #endif
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_RMM(RAWIspCamInfo const& rCamInfo __unused, IspNvramRegMgr & rIspRegMgr __unused, ISP_NVRAM_RMM_T& rRMM __unused)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rNR1.con = 0x%8x", rNR1.con);
    MY_LOG("rNR1.ct_con = 0x%8x", rNR1.ct_con);
    #endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_BOK(RAWIspCamInfo const& rCamInfo __unused, IspNvramRegMgr & rIspRegMgr __unused, ISP_NVRAM_BOK_T& rBOK __unused)
{
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCR.con = 0x%8x", rCCR.con);
    MY_LOG("rCCR.ylut = 0x%8x", rCCR.ylut);
    MY_LOG("rCCR.uvlut = 0x%8x", rCCR.uvlut);
    MY_LOG("rCCR.ylut2 = 0x%8x", rCCR.ylut2);
    MY_LOG("rCCR.sat_ctrl = 0x%8x", rCCR.sat_ctrl);
    MY_LOG("rCCR.uvlut_sp = 0x%8x", rCCR.uvlut_sp);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_RNR(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_RNR_T& rRNR)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rRNR;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_UDM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_UDM_T& rUDM)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rUDM;
    #if 0
    if(TUNING_FOR_AIS) {                          //0717forbuild
        if(IS_AIS) {
            rUDM.//rUDM.hf_comp.bits.DM_HF_LSC_GAIN0 = 8;
            rCFA.hf_comp.bits.DM_HF_LSC_GAIN1 = 8;
            rCFA.hf_comp.bits.DM_HF_LSC_GAIN2 = 8;
            rCFA.hf_comp.bits.DM_HF_LSC_GAIN3 = 8;
        }
    }
    #endif

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCFA.byp = 0x%8x", rCFA.byp);
    MY_LOG("rCFA.ed_flat = 0x%8x", rCFA.ed_flat);
    MY_LOG("rCFA.ed_nyq = 0x%8x", rCFA.ed_nyq);
    MY_LOG("rCFA.ed_step = 0x%8x", rCFA.ed_step);
    MY_LOG("rCFA.rgb_hf = 0x%8x", rCFA.rgb_hf);
    MY_LOG("rCFA.dot = 0x%8x", rCFA.dot);
    MY_LOG("rCFA.f1_act = 0x%8x", rCFA.f1_act);
    MY_LOG("rCFA.f2_act = 0x%8x", rCFA.f2_act);
    MY_LOG("rCFA.f3_act = 0x%8x", rCFA.f3_act);
    MY_LOG("rCFA.f4_act = 0x%8x", rCFA.f4_act);
    MY_LOG("rCFA.f1_l = 0x%8x", rCFA.f1_l);
    MY_LOG("rCFA.f2_l = 0x%8x", rCFA.f2_l);
    MY_LOG("rCFA.f3_l = 0x%8x", rCFA.f3_l);
    MY_LOG("rCFA.f4_l = 0x%8x", rCFA.f4_l);
    MY_LOG("rCFA.hf_rb = 0x%8x", rCFA.hf_rb);
    MY_LOG("rCFA.hf_gain = 0x%8x", rCFA.hf_gain);
    MY_LOG("rCFA.hf_comp = 0x%8x", rCFA.hf_comp);
    MY_LOG("rCFA.hf_coring_th = 0x%8x", rCFA.hf_coring_th);
    MY_LOG("rCFA.act_lut = 0x%8x", rCFA.act_lut);
    MY_LOG("rCFA.spare = 0x%8x", rCFA.spare);
    MY_LOG("rCFA.bb = 0x%8x", rCFA.bb);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*static MINT32 Complement2(MUINT32 value, MUINT32 digit)
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
}*/


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCM2(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCM_T& rCCMout)
{
    (void)rIspRegMgr;
    ISP_NVRAM_CCM_T rCCMIn;
    ::memcpy(&rCCMIn, &rCCMout, sizeof(ISP_NVRAM_CCM_T));

    if(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_toW ||
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW )
    {
        if( !m_CCM_DualPara_valid ){
            const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
            ::memcpy(m_CCM_DualPara, pSyncAEInitInfo->RGB2YCoef_main, sizeof(MUINT32)*3);
            m_CCM_DualPara_valid = MTRUE;
        }
        RGB2WMatrix( m_CCM_DualPara, rCamInfo.rAWBInfo.rCurrentAWBGain, rCCMout);
    }

    if(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview)
    {
        RGB2YLinear(rCCMIn, rCCMout);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_CCM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCM_T& rCCM)
{
    (void)rIspRegMgr;

    if(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Depth_toW)
    {
        if( !m_CCM_DualPara_valid ){
            const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
            ::memcpy(m_CCM_DualPara, pSyncAEInitInfo->RGB2YCoef_main, sizeof(MUINT32)*3);
            m_CCM_DualPara_valid = MTRUE;
        }
        RGB2WMatrix( m_CCM_DualPara, rCamInfo.rAWBInfo.rCurrentAWBGain, rCCM);
    }
}

MVOID
IspTuningCustom::
RGB2WMatrix(const MUINT32 L[], const AWB_GAIN_T& PGN, ISP_NVRAM_CCM_T& W)
{
    const int accuracy = 11;
    int mul = 1 << accuracy;
    float fL[3];
    int iL[3];
    fL[0] = (float)L[0] / 10000;
    fL[1] = (float)L[1] / 10000;
    fL[2] = (float)L[2] / 10000;
    // For PGN 1x = 512
    fL[0] = fL[0] * 512 / (float)PGN.i4R;
    fL[1] = fL[1] * 512 / (float)PGN.i4G;
    fL[2] = fL[2] * 512 / (float)PGN.i4B;
    iL[0] = (unsigned int)(fL[0] * mul);
    iL[1] = (unsigned int)(fL[1] * mul);
    iL[2] = (unsigned int)(fL[2] * mul);
    W.cnv_1.bits.G2G_CNV_00 = iL[0];
    W.cnv_1.bits.G2G_CNV_01 = iL[1];
    W.cnv_2.bits.G2G_CNV_02 = iL[2];
    W.cnv_3.bits.G2G_CNV_10 = iL[0];
    W.cnv_3.bits.G2G_CNV_11 = iL[1];
    W.cnv_4.bits.G2G_CNV_12 = iL[2];
    W.cnv_5.bits.G2G_CNV_20 = iL[0];
    W.cnv_5.bits.G2G_CNV_21 = iL[1];
    W.cnv_6.bits.G2G_CNV_22 = iL[2];
}


MVOID
IspTuningCustom::
RGB2YLinear(const ISP_NVRAM_CCM_T& inW, ISP_NVRAM_CCM_T& outW)
{
    //const int accuracy = 11;

    float fL[3];
    int iL[3];

    // transform to signed value
    int transW[9];
    transW[0] = inW.cnv_1.bits.G2G_CNV_00 > 4095 ? (int)inW.cnv_1.bits.G2G_CNV_00 - 8192 : inW.cnv_1.bits.G2G_CNV_00;
    transW[1] = inW.cnv_1.bits.G2G_CNV_01 > 4095 ? (int)inW.cnv_1.bits.G2G_CNV_01 - 8192 : inW.cnv_1.bits.G2G_CNV_01;
    transW[2] = inW.cnv_2.bits.G2G_CNV_02 > 4095 ? (int)inW.cnv_2.bits.G2G_CNV_02 - 8192 : inW.cnv_2.bits.G2G_CNV_02;
    transW[3] = inW.cnv_3.bits.G2G_CNV_10 > 4095 ? (int)inW.cnv_3.bits.G2G_CNV_10 - 8192 : inW.cnv_3.bits.G2G_CNV_10;
    transW[4] = inW.cnv_3.bits.G2G_CNV_11 > 4095 ? (int)inW.cnv_3.bits.G2G_CNV_11 - 8192 : inW.cnv_3.bits.G2G_CNV_11;
    transW[5] = inW.cnv_4.bits.G2G_CNV_12 > 4095 ? (int)inW.cnv_4.bits.G2G_CNV_12 - 8192 : inW.cnv_4.bits.G2G_CNV_12;
    transW[6] = inW.cnv_5.bits.G2G_CNV_20 > 4095 ? (int)inW.cnv_5.bits.G2G_CNV_20 - 8192 : inW.cnv_5.bits.G2G_CNV_20;
    transW[7] = inW.cnv_5.bits.G2G_CNV_21 > 4095 ? (int)inW.cnv_5.bits.G2G_CNV_21 - 8192 : inW.cnv_5.bits.G2G_CNV_21;
    transW[8] = inW.cnv_6.bits.G2G_CNV_22 > 4095 ? (int)inW.cnv_6.bits.G2G_CNV_22 - 8192 : inW.cnv_6.bits.G2G_CNV_22;

    // Note the accuarcy is increased from 9 to 11, the coefficient is sum to 4
    fL[0] = 1.196f * transW[0] + 2.348f * transW[3] + 0.456f * transW[6];
    fL[1] = 1.196f * transW[1] + 2.348f * transW[4] + 0.456f * transW[7];
    fL[2] = 1.196f * transW[2] + 2.348f * transW[5] + 0.456f * transW[8];

    iL[0] = fL[0] >= 0 ? int(fL[0] + 0.5f) : -int(-fL[0] + 0.5f);
    iL[1] = fL[1] >= 0 ? int(fL[1] + 0.5f) : -int(-fL[1] + 0.5f);
    iL[2] = fL[2] >= 0 ? int(fL[2] + 0.5f) : -int(-fL[2] + 0.5f);

    // transform to unsigned value
    iL[0] = iL[0] >= 0 ? (iL[0] > 4095 ? 4095 : iL[0]) : (iL[0] < -4096 ? 4096 : iL[0]+8192);
    iL[1] = iL[1] >= 0 ? (iL[1] > 4095 ? 4095 : iL[1]) : (iL[1] < -4096 ? 4096 : iL[1]+8192);
    iL[2] = iL[2] >= 0 ? (iL[2] > 4095 ? 4095 : iL[2]) : (iL[2] < -4096 ? 4096 : iL[2]+8192);

    outW.cnv_1.bits.G2G_CNV_00 = iL[0];
    outW.cnv_1.bits.G2G_CNV_01 = iL[1];
    outW.cnv_2.bits.G2G_CNV_02 = iL[2];
    outW.cnv_3.bits.G2G_CNV_10 = iL[0];
    outW.cnv_3.bits.G2G_CNV_11 = iL[1];
    outW.cnv_4.bits.G2G_CNV_12 = iL[2];
    outW.cnv_5.bits.G2G_CNV_20 = iL[0];
    outW.cnv_5.bits.G2G_CNV_21 = iL[1];
    outW.cnv_6.bits.G2G_CNV_22 = iL[2];
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_GGM(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_GGM_T& rGGM)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rGGM;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rGGM.lut_rb.lut[0] = 0x%8x", rGGM.lut_rb.lut[0]);
    MY_LOG("rGGM.lut_g.lut[0] = 0x%8x", rGGM.lut_g.lut[0]);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_ANR(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_ANR_T& rANR)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rANR;

    char InputValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.refine_ANR", InputValue, "0");
    //MUINT32 debugEn = atoi(InputValue);



//    if (debugEn) MY_LOG("rANR.con1.val(0x%08x)", rANR.con1.val);


#if 0

    if (rCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
            rANR.con1.bits.ANR1_ENC = 0;
            rANR.con1.bits.ANR1_ENY = 1;
            rANR.con1.bits.ANR_SCALE_MODE = 1;
            rANR.con1.bits.ANR1_MEDIAN_EN = 0;
            rANR.con1.bits.ANR1_LCE_LINK = 0;
            rANR.con1.bits.ANR1_TABLE_EN = 0;
            rANR.con1.bits.ANR1_TBL_PRC = 0;

            rANR.con2.bits.ANR1_IMPL_MODE = 2;
            rANR.con2.bits.ANR1_C_MED_EN = 0;
            rANR.con2.bits.ANR1_C_SM_EDGE = 1;
            rANR.con2.bits.ANR1_FLT_C = 0;
            rANR.con2.bits.ANR1_LR = 1;       // Set 1 to reduce contour
            rANR.con2.bits.ANR1_ALPHA = 4;    // when ANR1_LR == 1, ANR1_ALPHA = 4 is LR result, ANR1_ALPHA = 0 is original result

            rANR.yad1.bits.ANR1_CEN_GAIN_LO_TH = 1;
            rANR.yad1.bits.ANR1_CEN_GAIN_HI_TH = 5;
            rANR.yad1.bits.ANR1_K_LO_TH = 0;
            rANR.yad1.bits.ANR1_K_HI_TH = 9;
            rANR.yad1.bits.ANR1_K_TH_C = 8;

            rANR.yad2.bits.ANR1_PTY_VGAIN = 10;
            rANR.yad2.bits.ANR1_PTY_GAIN_TH = 10;

            rANR.lut1.bits.ANR1_Y_CPX1 = 40;
           rANR.lut1.bits.ANR1_Y_CPX2 = 100;
            rANR.lut1.bits.ANR1_Y_CPX3 = 160;

            rANR.lut2.bits.ANR1_Y_SCALE_CPY0 = 16;
            rANR.lut2.bits.ANR1_Y_SCALE_CPY1 = 16;
            rANR.lut2.bits.ANR1_Y_SCALE_CPY2 = 16;
            rANR.lut2.bits.ANR1_Y_SCALE_CPY3 = 8;

            rANR.lut3.bits.ANR1_Y_SCALE_SP0 = 0;
            rANR.lut3.bits.ANR1_Y_SCALE_SP1 = 0;
            rANR.lut3.bits.ANR1_Y_SCALE_SP2 = 0x1C;
            rANR.lut3.bits.ANR1_Y_SCALE_SP3 = 0x1E;

            // CAM_ANR_C4LUT1
            rANR.clut1.bits.ANR1_C_CPX1 = 40;
            rANR.clut1.bits.ANR1_C_CPX2 = 100;
            rANR.clut1.bits.ANR1_C_CPX3 = 160;

            // CAM_ANR_C4LUT2
            rANR.clut2.bits.ANR1_C_SCALE_CPY0 = 16;
            rANR.clut2.bits.ANR1_C_SCALE_CPY1 = 16;
            rANR.clut2.bits.ANR1_C_SCALE_CPY2 = 16;
            rANR.clut2.bits.ANR1_C_SCALE_CPY3 = 16;

            // CAM_ANR_C4LUT3
            rANR.clut3.bits.ANR1_C_SCALE_SP0 = 0;
            rANR.clut3.bits.ANR1_C_SCALE_SP1 = 0;
            rANR.clut3.bits.ANR1_C_SCALE_SP2 = 0;
            rANR.clut3.bits.ANR1_C_SCALE_SP3 = 0;

            // CAM_ANR_A4LUT2
            rANR.alut2.bits.ANR1_Y_ACT_CPY0 = 0;
            rANR.alut2.bits.ANR1_Y_ACT_CPY1 = 0;
            rANR.alut2.bits.ANR1_Y_ACT_CPY2 = 0;
            rANR.alut2.bits.ANR1_Y_ACT_CPY3 = 0;

            // CAM_ANR_A4LUT3
            rANR.alut3.bits.ANR1_Y_ACT_SP0 = 0;
            rANR.alut3.bits.ANR1_Y_ACT_SP1 = 0;
            rANR.alut3.bits.ANR1_Y_ACT_SP2 = 0;
            rANR.alut3.bits.ANR1_Y_ACT_SP3 = 0;

            // CAM_ANR_L4LUT1
            rANR.llut1.bits.ANR1_LCE_X1 = 64;
            rANR.llut1.bits.ANR1_LCE_X2 = 128;
            rANR.llut1.bits.ANR1_LCE_X3 = 192;

            // CAM_ANR_L4LUT2
            rANR.llut2.bits.ANR1_LCE_GAIN0 = 16;
            rANR.llut2.bits.ANR1_LCE_GAIN1 = 18;
            rANR.llut2.bits.ANR1_LCE_GAIN2 = 20;
            rANR.llut2.bits.ANR1_LCE_GAIN3 = 24;

            // CAM_ANR_L4LUT3
            rANR.llut3.bits.ANR1_LCE_SP0 = 4;
            rANR.llut3.bits.ANR1_LCE_SP1 = 4;
            rANR.llut3.bits.ANR1_LCE_SP2 = 8;
            rANR.llut3.bits.ANR1_LCE_SP3 = 4;

            rANR.pty.bits.ANR1_PTY1 = 12;
            rANR.pty.bits.ANR1_PTY2 = 34;
            rANR.pty.bits.ANR1_PTY3 = 58;
            rANR.pty.bits.ANR1_PTY4 = 72;

            rANR.cad.bits.ANR1_PTC_VGAIN = 10;
            rANR.cad.bits.ANR1_PTC_GAIN_TH = 6;
            rANR.cad.bits.ANR1_C_L_DIFF_TH = 28;

            rANR.ptc.bits.ANR1_PTC1 = 2;
            rANR.ptc.bits.ANR1_PTC2 = 3;
            rANR.ptc.bits.ANR1_PTC3 = 4;
            rANR.ptc.bits.ANR1_PTC4 = 6;

            rANR.lce.bits.ANR1_LCE_C_GAIN = 6;
            rANR.lce.bits.ANR1_LCE_SCALE_GAIN = 0;

            // CAM_ANR_MED1
            rANR.med1.bits.ANR1_COR_TH = 5;
            rANR.med1.bits.ANR1_COR_SL = 4;
            rANR.med1.bits.ANR1_MCD_TH = 5;
            rANR.med1.bits.ANR1_MCD_SL = 3;
            rANR.med1.bits.ANR1_LCL_TH = 24;

            // CAM_ANR_MED2
            rANR.med2.bits.ANR1_LCL_SL = 3;
            rANR.med2.bits.ANR1_LCL_LV = 16;
            rANR.med2.bits.ANR1_SCL_TH = 12;
            rANR.med2.bits.ANR1_SCL_SL = 3;
            rANR.med2.bits.ANR1_SCL_LV = 16;

            // CAM_ANR_MED3
            rANR.med3.bits.ANR1_NCL_TH = 255;
            rANR.med3.bits.ANR1_NCL_SL = 3;
            rANR.med3.bits.ANR1_NCL_LV = 16;
            rANR.med3.bits.ANR1_VAR = 3;
            rANR.med3.bits.ANR1_Y0 = 10;

            // CAM_ANR_MED4
            rANR.med4.bits.ANR1_Y1 = 13;
            rANR.med4.bits.ANR1_Y2 = 16;
            rANR.med4.bits.ANR1_Y3 = 19;
            rANR.med4.bits.ANR1_Y4 = 22;

            rANR.hp1.bits.ANR1_HP_A = 120;
            rANR.hp1.bits.ANR1_HP_B = 0x3C;
            rANR.hp1.bits.ANR1_HP_C = 0x19;
            rANR.hp1.bits.ANR1_HP_D = 0x9;
            rANR.hp1.bits.ANR1_HP_E = 0xB;

            rANR.hp2.bits.ANR1_HP_S1 = 0;
            rANR.hp2.bits.ANR1_HP_S2 = 0;
            rANR.hp2.bits.ANR1_HP_X1 = 0;
            rANR.hp2.bits.ANR1_HP_F = 0x2;

            rANR.hp3.bits.ANR1_HP_Y_GAIN_CLIP = 119;
            rANR.hp3.bits.ANR1_HP_Y_SP = 6;
            rANR.hp3.bits.ANR1_HP_Y_LO = 120;
            rANR.hp3.bits.ANR1_HP_CLIP = 0;

            rANR.acty.bits.ANR1_ACT_TH_Y = 16;
            rANR.acty.bits.ANR1_ACT_BLD_BASE_Y = 48;
            rANR.acty.bits.ANR1_ACT_SLANT_Y = 14;
            rANR.acty.bits.ANR1_ACT_BLD_TH_Y = 48;

            // CAM_ANR_ACT1 CAM+A64H
            rANR.act1.bits.ANR1_ACT_LCE_GAIN = 8;
            rANR.act1.bits.ANR1_ACT_SCALE_OFT = 0;
            rANR.act1.bits.ANR1_ACT_SCALE_GAIN = 0;
            rANR.act1.bits.ANR1_ACT_DIF_GAIN = 0;
            rANR.act1.bits.ANR1_ACT_DIF_LO_TH = 16;

            // CAM_ANR_ACT2 CAM+A64H
            rANR.act2.bits.ANR1_ACT_SIZE_GAIN = 0;
            rANR.act2.bits.ANR1_ACT_SIZE_LO_TH = 16;
            rANR.act2.bits.ANR1_COR_TH1 = 0;
            rANR.act2.bits.ANR1_COR_SL1 = 0;

            // CAM_ANR_ACT3 CAM+A64H
            rANR.act3.bits.ANR1_COR_ACT_TH = 0;
            rANR.act3.bits.ANR1_COR_ACT_SL1 = 0;
            rANR.act3.bits.ANR1_COR_ACT_SL2 = 0;

            rANR.actc.bits.ANR1_ACT_TH_C = 0;
            rANR.actc.bits.ANR1_ACT_BLD_BASE_C = 0;
            rANR.actc.bits.ANR1_ACT_SLANT_C = 0;
            rANR.actc.bits.ANR1_ACT_BLD_TH_C = 0;


    }
#endif

#if 0
    if(TUNING_FOR_AIS) {
        // enable sl2b for ais stage3/4
        if(IS_AIS) {
            if (rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR  )
            {
                    rANR.con1.bits.ANR_LCE_LINK = 1;
            }
        }

        //stage1 use high iso, stage2/3/4 use low iso
        // use low iso luma anr for ais stage3/4
        // use high iso chroma anr for ais stage3/4 <- this
        if(IS_AIS) {
            if (rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR   )
            {
                MUINT32 stage = AIS_Profile2Stage(rCamInfo.eIspProfile);

                MUINT32 aisIso = rCamInfo.u4ISOValue;
                MUINT32 normalIso = rCamInfo.rAEInfo.u4OrgRealISOValue;
                MUINT32 ynrIso = WEIGHTING(normalIso, aisIso, AIS_NORMAL_YNR_RATIO[stage-1]);
                MUINT32 cnrIso = WEIGHTING(normalIso, aisIso, AIS_NORMAL_CNR_RATIO[stage-1]);

                MINT32 aisIndex = map_ISO_value_to_index(aisIso);
                MINT32 normalIndex = map_ISO_value_to_index(normalIso);
                MINT32 ynrIndex = map_ISO_value_to_index(ynrIso);

                //base on YNR
                MINT32 deltaCnr = map_ISO_value_to_index(cnrIso) - ynrIndex;

                ISP_NVRAM_ANR_T rAnrSettingForCnr = rIspRegMgr.getANR(rIspRegMgr.getIdx_ANR() + deltaCnr);

                if (debugEn) MY_LOG("rANR.con1.val(0x%08x), rAnrSettingForCnr.con1.val(0x%08x)", rANR.con1.val, rAnrSettingForCnr.con1.val);
                //con1
                rANR.con1.bits.ANR_ENC = rAnrSettingForCnr.con1.bits.ANR_ENC;
                rANR.con1.bits.ANR_SCALE_MODE = rAnrSettingForCnr.con1.bits.ANR_SCALE_MODE;
                //con3
                rANR.con3.bits.ANR_C_SM_EDGE = rAnrSettingForCnr.con3.bits.ANR_C_SM_EDGE;
                //cad(all)
                //rANR.cad.bits.ANR_PTC_VGAIN = rAnrSettingForCnr.cad.bits.ANR_PTC_VGAIN;
                //rANR.cad.bits.ANR_PTC_GAIN_TH = rAnrSettingForCnr.cad.bits.ANR_PTC_GAIN_TH;
                //rANR.cad.bits.ANR_C_L_DIFF_TH = rAnrSettingForCnr.cad.bits.ANR_C_L_DIFF_TH;
                rANR.cad.val = rAnrSettingForCnr.cad.val;
                //ptc(all)
                //rANR.ptc.bits.ANR_PTC1 = rAnrSettingForCnr.ptc.bits.ANR_PTC1;
                //rANR.ptc.bits.ANR_PTC2 = rAnrSettingForCnr.ptc.bits.ANR_PTC2;
                //rANR.ptc.bits.ANR_PTC3 = rAnrSettingForCnr.ptc.bits.ANR_PTC3;
                //rANR.ptc.bits.ANR_PTC4 = rAnrSettingForCnr.ptc.bits.ANR_PTC4;
                rANR.ptc.val = rAnrSettingForCnr.ptc.val;
                //lce1
                rANR.lce1.bits.ANR_LCE_C_GAIN = rAnrSettingForCnr.lce1.bits.ANR_LCE_C_GAIN;
                //lce2(all)
                //rANR.lce2.bits.ANR_LCE_GAIN0 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN0;
                //rANR.lce2.bits.ANR_LCE_GAIN1 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN1;
                //rANR.lce2.bits.ANR_LCE_GAIN2 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN2;
                //rANR.lce2.bits.ANR_LCE_GAIN3 = rAnrSettingForCnr.lce2.bits.ANR_LCE_GAIN3;
                rANR.lce2.val = rAnrSettingForCnr.lce2.val;
                //actc(all)
                //rANR.actc.bits.ANR_ACT_TH_C = rAnrSettingForCnr.actc.bits.ANR_ACT_TH_C;
                //rANR.actc.bits.ANR_ACT_BLD_BASE_C = rAnrSettingForCnr.actc.bits.ANR_ACT_BLD_BASE_C;
                //rANR.actc.bits.ANR_ACT_SLANT_C = rAnrSettingForCnr.actc.bits.ANR_ACT_SLANT_C;
                //rANR.actc.bits.ANR_ACT_BLD_TH_C = rAnrSettingForCnr.actc.bits.ANR_ACT_BLD_TH_C;
                rANR.actc.val = rAnrSettingForCnr.actc.val;
            }
        }
    }

    #endif

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rANR.con1 = 0x%8x", rANR.con1);
    MY_LOG("rANR.con2 = 0x%8x", rANR.con2);
    MY_LOG("rANR.con3 = 0x%8x", rANR.con3);
    MY_LOG("rANR.yad1 = 0x%8x", rANR.yad1);
    MY_LOG("rANR.yad2 = 0x%8x", rANR.yad2);
    MY_LOG("rANR.lut1 = 0x%8x", rANR.lut1);
    MY_LOG("rANR.lut2 = 0x%8x", rANR.lut2);
    MY_LOG("rANR.lut3 = 0x%8x", rANR.lut3);
    MY_LOG("rANR.pty = 0x%8x", rANR.pty);
    MY_LOG("rANR.cad = 0x%8x", rANR.cad);
    MY_LOG("rANR.ptc = 0x%8x", rANR.ptc);
    MY_LOG("rANR.lce1 = 0x%8x", rANR.lce1);
    MY_LOG("rANR.lce2 = 0x%8x", rANR.lce2);
    MY_LOG("rANR.hp1 = 0x%8x", rANR.hp1);
    MY_LOG("rANR.hp2 = 0x%8x", rANR.hp2);
    MY_LOG("rANR.hp3 = 0x%8x", rANR.hp3);
    MY_LOG("rANR.acty = 0x%8x", rANR.acty);
    MY_LOG("rANR.actc = 0x%8x", rANR.actc);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_ANR2(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_ANR2_T& rANR)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rANR;
    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCR.con = 0x%8x", rCCR.con);
    MY_LOG("rCCR.ylut = 0x%8x", rCCR.ylut);
    MY_LOG("rCCR.uvlut = 0x%8x", rCCR.uvlut);
    MY_LOG("rCCR.ylut2 = 0x%8x", rCCR.ylut2);
    MY_LOG("rCCR.sat_ctrl = 0x%8x", rCCR.sat_ctrl);
    MY_LOG("rCCR.uvlut_sp = 0x%8x", rCCR.uvlut_sp);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_CCR(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_CCR_T& rCCR)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rCCR;
/*
    if (rCamInfo.eIspProfile == EIspProfile_VFB_PostProc) {
       rCCR.con.bits.CCR_EN = 0;
    }
*/

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rCCR.con = 0x%8x", rCCR.con);
    MY_LOG("rCCR.ylut = 0x%8x", rCCR.ylut);
    MY_LOG("rCCR.uvlut = 0x%8x", rCCR.uvlut);
    MY_LOG("rCCR.ylut2 = 0x%8x", rCCR.ylut2);
    MY_LOG("rCCR.sat_ctrl = 0x%8x", rCCR.sat_ctrl);
    MY_LOG("rCCR.uvlut_sp = 0x%8x", rCCR.uvlut_sp);
    #endif
}

MVOID
IspTuningCustom::
refine_ABF(RAWIspCamInfo const& rCamInfo __unused, IspNvramRegMgr & rIspRegMgr __unused, ISP_NVRAM_ABF_T& rABF __unused)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_HFG(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_HFG_T& rHFG)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rHFG;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_EE(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_EE_T& rEE)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rEE;
     #if 0

     if(TUNING_FOR_AIS) {
        if(IS_AIS) {
            rEE.glut_ctrl_07.bits.SEEE_GLUT_LLINK_EN = 0;
        }
    }


    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    MY_LOG("rEE.srk_ctrl = 0x%8x", rEE.srk_ctrl);
    MY_LOG("rEE.clip_ctrl = 0x%8x", rEE.clip_ctrl);
    MY_LOG("rEE.flt_ctrl_1 = 0x%8x", rEE.flt_ctrl_1);
    MY_LOG("rEE.flt_ctrl_2 = 0x%8x", rEE.flt_ctrl_2);
    MY_LOG("rEE.glut_ctrl_01 = 0x%8x", rEE.glut_ctrl_01);
    MY_LOG("rEE.glut_ctrl_02 = 0x%8x", rEE.glut_ctrl_02);
    MY_LOG("rEE.glut_ctrl_03 = 0x%8x", rEE.glut_ctrl_03);
    MY_LOG("rEE.glut_ctrl_04 = 0x%8x", rEE.glut_ctrl_04);
    MY_LOG("rEE.glut_ctrl_05 = 0x%8x", rEE.glut_ctrl_05);
    MY_LOG("rEE.glut_ctrl_06 = 0x%8x", rEE.glut_ctrl_06);
    MY_LOG("rEE.edtr_ctrl = 0x%8x", rEE.edtr_ctrl);
    MY_LOG("rEE.glut_ctrl_07 = 0x%8x", rEE.glut_ctrl_07);
    MY_LOG("rEE.glut_ctrl_08 = 0x%8x", rEE.glut_ctrl_08);
    MY_LOG("rEE.glut_ctrl_09 = 0x%8x", rEE.glut_ctrl_09);
    MY_LOG("rEE.glut_ctrl_10 = 0x%8x", rEE.glut_ctrl_10);
    MY_LOG("rEE.glut_ctrl_11 = 0x%8x", rEE.glut_ctrl_11);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Use 3DNR for frame rate improvement
// Ratio|
//  100%|         /-----
//      |        /
//      |       /
//      |      /
//      |     /
//   0% +----|----|------
//      TH_LOW  TH_HIGH
#define LIMITER(Input, LowerBound, UpperBound)  do { if (Input > UpperBound){Input = UpperBound;} if (Input < LowerBound){Input = LowerBound;} } while (0)

// Note: X2 must be larger than or equal to X1.
inline MINT32 Nr3dLmtInterpolation(MINT32 TargetX, MINT32 X1, MINT32 Y1, MINT32 X2, MINT32 Y2)
{
    MINT32 TargetY = 0;
    MINT32 TempValue = 0;
    MINT32 RoundingValue = 0;

    if (X1 == X2)
    {
        TargetY = Y1;
    }
    else if ( TargetX <= X1 )
    {
        TargetY = Y1;
    }
    else if ( TargetX >= X2 )
    {
        TargetY = Y2;
    }
    else    // if (X1 <= TargetX <= X2), then interpolation.
    {
        TempValue = (TargetX - X1) * (Y2 - Y1);
        RoundingValue = (X2 - X1) >> 1;
        TargetY = (TempValue + RoundingValue) / (X2 - X1) + Y1;
    }

    return TargetY;

}


MVOID
IspTuningCustom::
refine_NR3D(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_NR3D_T& rNR3D)
{

    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rNR3D;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_MFB(RAWIspCamInfo const& rCamInfo __unused, IspNvramRegMgr & rIspRegMgr __unused, ISP_NVRAM_MFB_T& rMFB __unused)
{
#if 0
    (void)rIspRegMgr;

    if(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_MFNR_After_Blend) {
        rMFB.con.bits.BLD_LL_DB_EN = 0;
        rMFB.con.bits.BLD_LL_BRZ_EN = 0;
    }

    MINT32 debugDump = property_get_int32("vendor.debug.mfb.log", 0);

    MY_LOG_IF(debugDump, "%s()\n", __FUNCTION__);
    MY_LOG_IF(debugDump, "rMFB.profile = %d", rCamInfo.rMapping_Info.eIspProfile);
    MY_LOG_IF(debugDump, "rMFB.con = 0x%8x", rMFB.con.val);
    MY_LOG_IF(debugDump, "rMFB.ll_con1 = 0x%8x", rMFB.ll_con1.val);
    MY_LOG_IF(debugDump, "rMFB.ll_con2 = 0x%8x", rMFB.ll_con2.val);
    MY_LOG_IF(debugDump, "rMFB.ll_con4 = 0x%8x", rMFB.ll_con4.val);
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_MIXER3(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_MIXER3_T& rMIXER3)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rMIXER3;

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    // TODO: Add your code below...

    rMIXER3.ctrl_0.bits.MIX3_WT_SEL = 1;
    rMIXER3.ctrl_0.bits.MIX3_B0 = 0;
    rMIXER3.ctrl_0.bits.MIX3_B1 = 0xFF;
    rMIXER3.ctrl_0.bits.MIX3_DT = 1;

    rMIXER3.ctrl_1.bits.MIX3_M0 = 0;
    rMIXER3.ctrl_1.bits.MIX3_M1 = 0xFF;

    MY_LOG("rMIXER3.ctrl_0 = 0x%8x", rMIXER3.ctrl_0);
    MY_LOG("rMIXER3.ctrl_1 = 0x%8x", rMIXER3.ctrl_1);
    MY_LOG("rMIXER3.spare = 0x%8x", rMIXER3.spare);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
refine_LCE(RAWIspCamInfo const& rCamInfo, IspNvramRegMgr & rIspRegMgr, ISP_NVRAM_LCE_T& rLCE)
{
    (void)rCamInfo;
    (void)rIspRegMgr;
    (void)rLCE;

    #if 0
    MY_LOG("%s()\n", __FUNCTION__);
    MY_LOG("rLCE.qua = 0x%8x", rLCE.qua);
    #endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EIndex_CCM_T
IspTuningCustom::
evaluate_CCM_index(RAWIspCamInfo const& rCamInfo, EIndex_CCM_T const ePrvIdx)
{
    MINT32 debugDump = property_get_int32("vendor.debug.custom.ccm", 0);



    MY_LOG_IF(debugDump, "%s()\n", __FUNCTION__);

    MY_LOG_IF(debugDump,
        "[+evaluate_CCM_index]"
        "(eIdx_CCM, i4CCT, i4FluorescentIndex)=(%d, %d, %d)"
        , ePrvIdx
        , rCamInfo.rAWBInfo.i4CCT
        , rCamInfo.rAWBInfo.i4FluorescentIndex);

    EIndex_CCM_T eIdx_CCM_new = ePrvIdx;

//    -----------------|---|---|--------------|---|---|------------------
//                                THA TH1 THB              THC TH2  THD

    MINT32 const THA = 3318;
    MINT32 const TH1 = 3484;
    MINT32 const THB = 3667;
    MINT32 const THC = 4810;
    //MINT32 const TH2 = 5050;
    MINT32 const THD = 5316;
    MINT32 const F_IDX_TH1 = 25;
    MINT32 const F_IDX_TH2 = -25;

    switch  (ePrvIdx)
    {
    case eIDX_CCM_TL84:
        if  ( rCamInfo.rAWBInfo.i4CCT < THB )
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
        {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex < F_IDX_TH2 )
                eIdx_CCM_new = eIDX_CCM_CWF;
            else
                eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_CWF:
        if  ( rCamInfo.rAWBInfo.i4CCT < THA )
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT < THD )
        {
            if  ( rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH1 )
                eIdx_CCM_new = eIDX_CCM_TL84;
            else
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        break;
    case eIDX_CCM_D65:
        if  ( rCamInfo.rAWBInfo.i4CCT > THC )
        {
            eIdx_CCM_new = eIDX_CCM_D65;
        }
        else if ( rCamInfo.rAWBInfo.i4CCT > TH1 )
        {
            if(rCamInfo.rAWBInfo.i4FluorescentIndex > F_IDX_TH2)
                eIdx_CCM_new = eIDX_CCM_TL84;
            else
                eIdx_CCM_new = eIDX_CCM_CWF;
        }
        else
        {
            eIdx_CCM_new = eIDX_CCM_TL84;
        }
        break;
    default:
        break;
    }

    if  ( ePrvIdx != eIdx_CCM_new )
    {
        MY_LOG_IF(debugDump,
            "[-evaluate_CCM_index] CCM Idx(old,new)=(%d,%d)"
            , ePrvIdx, eIdx_CCM_new
        );
    }

    return  eIdx_CCM_new;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_capture_in_color(RAWIspCamInfo const& rCamInfo)
{
    if (// normal capture
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Capture ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Capture_Capture_ZOOM1 || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Capture_Capture_ZOOM2 || \
        // flash capture
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Flash_Capture ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Flash_Capture_ZOOM1 || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Flash_Capture_ZOOM2 || \
        // HDR
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Capture || \
        //rCamInfo.eIspProfile == EIspProfile_Auto_zHDR_Capture || \
        //rCamInfo.eIspProfile == EIspProfile_Auto_mHDR_Capture || \
        // N3D
        //rCamInfo.eIspProfile == EIspProfile_N3D_MFHR_After_Blend || \
        //rCamInfo.eIspProfile == EIspProfile_N3D_Denoise || \
        // MFNR
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_MFNR_After_Blend || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_MFNR_After_ZOOM1 || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_MFNR_After_ZOOM2 || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_After_Blend
        // HDR Flash Capture
        //rCamInfo.eIspProfile == EIspProfile_zHDR_Flash_Capture
        )
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_isp_interpolation(RAWIspCamInfo const& rCamInfo)
{
    (void)rCamInfo;
#if 0
    if(TUNING_FOR_AIS) {
        if(IS_AIS) {
            if(
                   rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Capture_EE_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off
                || rCamInfo.eIspProfile == EIspProfile_MFB_Blending_All_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_MFB_PostProc_Mixing_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Capture_EE_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_EE_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_Blending_All_Off_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing
                || rCamInfo.eIspProfile == EIspProfile_VSS_MFB_PostProc_Mixing_SWNR

            )
            {
                return MFALSE;
            }
       }
    }
#endif
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_flc(RAWIspCamInfo const& rCamInfo)
{
    (void)rCamInfo;
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
IspTuningCustom::
is_HDRTuning(RAWIspCamInfo const& rCamInfo){

    if((rCamInfo.rMapping_Info.eIspProfile == EIspProfile_iHDR_Preview)                     ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Preview)                     ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Preview)                     ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_iHDR_Video)                       ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Video)                       ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Video)                       ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_iHDR_Preview_VSS)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Preview_VSS)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Preview_VSS)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_iHDR_Video_VSS)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Video_VSS)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Video_VSS)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture)                     ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Capture)                     ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_iHDR_Preview)                ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Preview)                ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Preview)                ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_iHDR_Video)                  ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Video)                  ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Video)                  ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_iHDR_Preview_VSS)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Preview_VSS)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Preview_VSS)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_iHDR_Video_VSS)              ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Video_VSS)              ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Video_VSS)              ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Capture)                ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Capture)                ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_Before_Blend)   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_Single)         ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_MFB)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_After_Blend)    ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_iHDR_Preview)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_zHDR_Preview)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_mHDR_Preview)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_iHDR_Video)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_zHDR_Video)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_mHDR_Video)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_iHDR_Preview)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_zHDR_Preview)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_mHDR_Preview)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_iHDR_Video)              ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_zHDR_Video)              ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_mHDR_Video)){
       //(rCamInfo.eIspProfile == EIspProfile_zHDR_Flash_Capture)){
       return MTRUE;
    }

    return MFALSE;

}
MBOOL
IspTuningCustom::
is_ZHDRTuning(RAWIspCamInfo const& rCamInfo){
       if ((rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Preview)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Video)                       ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Preview_VSS)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Video_VSS)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Preview)                     ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Video)                       ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Preview_VSS)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Video_VSS)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Video_VSS)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture)                     ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Preview)                ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Video)                  ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Preview_VSS)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Video_VSS)              ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Capture)                ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_Before_Blend)   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_Single)         ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_MFB)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_After_Blend)    ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_zHDR_Preview)                 ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_zHDR_Video)                   ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_zHDR_Preview)            ||
       (rCamInfo.rMapping_Info.eIspProfile == EIspProfile_EIS_Auto_zHDR_Video)){
       return MTRUE;
       }
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*******************************************************************************
*
* eIdx_Shading_CCT_old:
*   [in] the previous color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
* i4CCT:
*   [in] the current color temperature from 3A.
*
*
* return:
*   [out] the current color temperature index
*           eIDX_Shading_CCT_ALight
*           eIDX_Shading_CCT_CWF
*           eIDX_Shading_CCT_D65
*
*******************************************************************************/
EIndex_Shading_CCT_T
IspTuningCustom::
evaluate_Shading_CCT_index  (
        RAWIspCamInfo const& rCamInfo
)   const
{
    MUINT32 EN_LSC_LOG_TSF_RUN = (1<<6);
    MUINT32 m_u4LogEn = property_get_int32("vendor.debug.lsc_mgr.log", 0);
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    MINT32 i4CCT = rCamInfo.rAWBInfo.i4CCT;

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = rCamInfo.eIdx_Shading_CCT;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

    MINT32 const THL1 = 3257;
    MINT32 const THH1 = 3484;
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const THL2 = 4673;
    MINT32 const THH2 = 5155;
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

    switch  (rCamInfo.eIdx_Shading_CCT)
    {
    case eIDX_Shading_CCT_ALight:
        if  ( i4CCT < THH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT <  TH2)
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_CWF:
        if  ( i4CCT < THL1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THH2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_D65:
        if  ( i4CCT < TH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THL2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    default:
        break;
    }

    if  ( rCamInfo.eIdx_Shading_CCT != eIdx_Shading_CCT_new )
    {
        MY_LOG_IF(fgLogEn,
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , rCamInfo.eIdx_Shading_CCT, eIdx_Shading_CCT_new,i4CCT
        );
    }

    return  eIdx_Shading_CCT_new;
}

MVOID
IspTuningCustom::
reset_ISO_SmoothBuffer()
{
    MUINT32 EN_LSC_LOG_TSF_RUN = (1<<6);
    MUINT32 m_u4LogEn = property_get_int32("vendor.debug.lsc_mgr.log", 0);
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    total_RA_num_frames_= 0;
    MY_LOG_IF(fgLogEn, "reset_ISO total_RA_num_frames_=0");
    memset(ISO_Buffer_, 6, sizeof(ISO_Buffer_));
    MY_LOG_IF(fgLogEn, "[%s] total_RA_num_frames_(%d)", __FUNCTION__, total_RA_num_frames_ );
    MY_LOG_IF(fgLogEn, "[%s] ISO_Buffer_[] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n}", __FUNCTION__,
        ISO_Buffer_[0], ISO_Buffer_[1], ISO_Buffer_[2], ISO_Buffer_[3], ISO_Buffer_[4],
        ISO_Buffer_[5], ISO_Buffer_[6], ISO_Buffer_[7], ISO_Buffer_[8], ISO_Buffer_[9] );
}

static MINT32 ratioMapping(MINT32 i4Iso)
{
#define LERP(x, lo_x, lo_y, hi_x, hi_y)\
    (((hi_x) - (x))*(lo_y) + ((x) - (lo_x))*(hi_y)) / ((hi_x) - (lo_x))

    static const MINT32 iso[10] =
    {100, 200, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200};

    static const MINT32 rto[10] =
    //{24, 22, 20, 18, 16, 14, 12, 10, 8, 6}; //Tower modify for iso1600 Noise 2014-12-26
    //{30, 28, 26, 24, 22, 20, 18, 16, 14, 12};
    {32, 32, 32, 30, 28, 26, 24, 22, 20, 18};

    MINT32 i = 0;
    MINT32 i4Rto = 32;

    if (i4Iso < iso[0])
    {
        i4Rto = rto[0];
    }
    else if (i4Iso >= iso[9])
    {
        i4Rto = rto[9];
    }
    else
    {
        for (i = 1; i < 10; i++)
        {
            if (i4Iso < iso[i])
                break;
        }
        i4Rto = LERP(i4Iso, iso[i-1], rto[i-1], iso[i], rto[i]);
    }
    return i4Rto;
}

static MINT32 ratioMappingByEV(MINT32 i4Rto, float i4EV, MUINT32 u4SensorId)
{
#define LERP(x, lo_x, lo_y, hi_x, hi_y)\
    (((hi_x) - (x))*(lo_y) + ((x) - (lo_x))*(hi_y)) / ((hi_x) - (lo_x))

#define mappingTableSize (10)
    float adTBL_EV[mappingTableSize];
    float fTBL_RA[mappingTableSize];

    static const float adTBL_EV_default[mappingTableSize] =
        { -8,  -7,  -6,  -5,   -4,   -2,   -1,    0,    1,  2};
    static const float fTBL_RA_default[mappingTableSize] =
        {  0,   0,   0,   0,    0,  0.2,  0.35,  0.5,   0.8, 1};

    switch(u4SensorId)
    {
        case 0x8d1:
        default:
            memcpy(adTBL_EV, adTBL_EV_default, sizeof(float)*mappingTableSize);
            memcpy(fTBL_RA,  fTBL_RA_default,  sizeof(float) *mappingTableSize);
            break;
    }

    MINT32 i = 0;
    float final_fTBL_RA = 1;

    if (i4EV < adTBL_EV[0])
    {
        final_fTBL_RA = fTBL_RA[0];
    }
    else if (i4EV >= adTBL_EV[9])
    {
        final_fTBL_RA = fTBL_RA[9];
    }
    else
    {
        for (i = 1; i < mappingTableSize - 1; i++)
        {
            if (i4EV < adTBL_EV[i])
                break;
        }
        final_fTBL_RA = LERP((float)i4EV, (float)adTBL_EV[i-1], (float)fTBL_RA[i-1], (float)adTBL_EV[i], (float)fTBL_RA[i]);
    }

    return (((float)(i4Rto))*final_fTBL_RA);
}

//char output_log[256];
//per index is 0.1 EV
MUINT32 power_2_lshift20_positive[46]=
{1048576, 1123836, 1204498, 1290948, 1383604, 1482910, 1589344, 1703417, 1825677, 1956712, 2097152, 2247672, 2408995, 2581897, 2767209, 2965821, 3178688, 3406833, 3651354, 3913424, 4194304, 4495344, 4817990, 5163794, 5534417, 5931642, 6357376, 6813667, 7302707, 7826848, 8388608, 8990687, 9635980, 10327588, 11068835, 11863283, 12714752, 13627334, 14605415, 15653696, 16777216, 17981375, 19271960, 20655176, 22137669, 23726566};
MUINT32  power_2_lshift20_negative[46]=
{1048576, 978356, 912838, 851708, 794672, 741455, 691802, 645474, 602249, 561918, 524288, 489178, 456419, 425854, 397336, 370728, 345901, 322737, 301124, 280959, 262144, 244589, 228210, 212927, 198668, 185364, 172951, 161369, 150562, 140479, 131072, 122295, 114105, 106464, 99334, 92682, 86475, 80684, 75281, 70240, 65536, 61147, 57052, 53232, 49667, 46341};


MINT32
IspTuningCustom::
evaluate_Shading_Ratio  (
            RAWIspCamInfo const& rCamInfo,
            MBOOL isSuperNightMode
)
{
    /*
        Sample code for evaluate shading ratio.
        The shading ratio is an integer ranging from 0(0%) to 32(100%).
        All informations can be obtained via rCamInfo.
        The following sample code shows a shading ratio evaluated by ISO value with temporal smoothness.
    */

    MUINT32 EN_LSC_LOG_TSF_RUN = (1<<6);
    MUINT32 m_u4LogEn = property_get_int32("vendor.debug.lsc_mgr.log", 0);
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    MINT32 Avg_Frm_Cnt = 5;
    //MINT32 i = 0;
    MINT32 i4Rto = 8; //32;
    MINT32 i4Iso = rCamInfo.rAEInfo.u4RealISOValue;

    int idx = total_RA_num_frames_ % Avg_Frm_Cnt;
    //int *p_global_Ra = ISO_Buffer_;
    int n_frames, avgISO;

    ISO_Buffer_[idx] = i4Iso;

    // to prevent total frames overflow
    if (total_RA_num_frames_ >= 65535){
        total_RA_num_frames_ = 0;
    }
    total_RA_num_frames_++;
    if (total_RA_num_frames_ < 0){
        avgISO = 32;
        MY_LOG_IF(fgLogEn, "[%s] first avgISO = %d\n", __FUNCTION__, avgISO);
    } else {
        // smooth
        n_frames = ( total_RA_num_frames_ <  Avg_Frm_Cnt) ? (total_RA_num_frames_) : (Avg_Frm_Cnt);
        avgISO = 0;
        for (int k = 0; k < n_frames; k++) {
            avgISO += ISO_Buffer_[k];
        }
        avgISO /= n_frames;
        MY_LOG_IF(fgLogEn, "[%s] ISO_Buffer_[] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n}", __FUNCTION__,
        ISO_Buffer_[0], ISO_Buffer_[1], ISO_Buffer_[2], ISO_Buffer_[3], ISO_Buffer_[4],
        ISO_Buffer_[5], ISO_Buffer_[6], ISO_Buffer_[7], ISO_Buffer_[8], ISO_Buffer_[9] );
        MY_LOG_IF(fgLogEn, "[%s] avgISO = %d", __FUNCTION__, avgISO);
        if (rCamInfo.rMapping_Info.eFlash == EFlash_Yes)
        {
            i4Rto = ratioMapping(i4Iso);
            MY_LOG("[%s] Main flash iso(%d), ratio(%d)", __FUNCTION__, i4Iso, i4Rto);
        }
        else
        {
            ////    debug and expreiment only //////////////
            int OutSideAvgY = 0;
            int OutSideCnt = 0;

            for(int i=0;i<AE_BLOCK_NO;i++)
            {
                for(int j=0;j<AE_BLOCK_NO;j++)
                {
                    if(i==0 || j==0 || i==AE_BLOCK_NO-1 || j==AE_BLOCK_NO-1)
                    {
                        OutSideAvgY += rCamInfo.rAEInfo.pu4AEBlock[i][j];
                        OutSideCnt++;
                    }
                }
            }
            OutSideAvgY = OutSideAvgY/OutSideCnt;
            ////    debug and expreiment only //////////////

            int lsc_new_version = 0;
            //char value[128] = {0};
            //property_get("vendor.debug.lsc.new_version", value, "0");
            //lsc_new_version = atoi(value);

            lsc_new_version = rCamInfo.rAEInfo.bEnableRAFastConverge;
            MY_LOG_IF(fgLogEn,  "lsc_new_version=%d", lsc_new_version);

            if(lsc_new_version == 0)
            {
                i4Rto = ratioMapping(avgISO);
            }
            else
            {
            //int MAX_RA = 32;

            //int i, j;
            int CWV1;
            //int CWV2;
            int RA1 = 0;
            int RA2 = 0;
            int Dst_Ratio;
            int SlowLSC;
            int NonLinearLSC;
            float RA_Coeff = 0.0;
            //int CWV1_Multiply_WeightSum;
            float LSC_GAIN_TABLE[AE_BLOCK_NO][AE_BLOCK_NO];
            //int Pure_AE_Block[AE_BLOCK_NO][AE_BLOCK_NO];
            //int Pure_New_AE_Block[AE_BLOCK_NO][AE_BLOCK_NO];
            //int Pure_New_Weight_AE_Block[AE_BLOCK_NO][AE_BLOCK_NO];

            //static int CrntRatio = 0;
            static int PreFrame1Ratio = 0;
            static int PreFrame2Ratio = 0;
            static int PreFrame3Ratio = 0;
            static int PreFrame4Ratio = 0;


            LSC_GAIN_TABLE[0][0] = 1.0;
            LSC_GAIN_TABLE[0][1] = 1.0;
            LSC_GAIN_TABLE[0][2] = 1.0;
            LSC_GAIN_TABLE[0][3] = 1.0;
            LSC_GAIN_TABLE[0][4] = 1.0;

            LSC_GAIN_TABLE[1][0] = 1.0;
            LSC_GAIN_TABLE[1][1] = 1.0;
            LSC_GAIN_TABLE[1][2] = 1.0;
            LSC_GAIN_TABLE[1][3] = 1.0;
            LSC_GAIN_TABLE[1][4] = 1.0;

            LSC_GAIN_TABLE[2][0] = 1.0;
            LSC_GAIN_TABLE[2][1] = 1.0;
            LSC_GAIN_TABLE[2][2] = 1.0;
            LSC_GAIN_TABLE[2][3] = 1.0;
            LSC_GAIN_TABLE[2][4] = 1.0;

            LSC_GAIN_TABLE[3][0] = 1.0;
            LSC_GAIN_TABLE[3][1] = 1.0;
            LSC_GAIN_TABLE[3][2] = 1.0;
            LSC_GAIN_TABLE[3][3] = 1.0;
            LSC_GAIN_TABLE[3][4] = 1.0;

            LSC_GAIN_TABLE[4][0] = 1.0;
            LSC_GAIN_TABLE[4][1] = 1.0;
            LSC_GAIN_TABLE[4][2] = 1.0;
            LSC_GAIN_TABLE[4][3] = 1.0;
            LSC_GAIN_TABLE[4][4] = 1.0;

            Dst_Ratio = ratioMapping(rCamInfo.rAEInfo.u4RealISOValue);

            RA_Coeff = 0.0;
            RA1 = PreFrame2Ratio;
            CWV1 = rCamInfo.rAEInfo.u4MgrCWValue;

            if( ( CWV1>=230 || CWV1<=20) || rCamInfo.rAEInfo.TgCtrlRight==MTRUE  )
            {
                NonLinearLSC = 1;
            }
            else
            {
                NonLinearLSC = 0;
            }

            if( abs(rCamInfo.rAEInfo.i4deltaIndex) > 16*rCamInfo.rAEInfo.u4AEFinerEVIdxBase &&  NonLinearLSC==0 &&  !rCamInfo.rAEInfo.bAEStable)
            {
                /*
                MUINT32 *power_tbl;
                MUINT32 abs_delta_idx = abs(rCamInfo.rAEInfo.i4deltaIndex);

                if(rCamInfo.rAEInfo.i4deltaIndex>=0)
                    power_tbl = power_2_lshift20_positive;
                else
                    power_tbl = power_2_lshift20_negative;


                CWV1_Multiply_WeightSum = CWV1*rCamInfo.rAEInfo.u4WeightingSum;

                for(i=0;i<AE_BLOCK_NO;i++)
                {
                    for(j=0;j<AE_BLOCK_NO;j++)
                    {
                        float lsc_gain = 1.0 + (float)RA1*(LSC_GAIN_TABLE[i][j]-1.0)/(float)MAX_RA; //­ì¥»ªº­?¥h­¼ 1.0+(ratio/MAX_RA)*(gain-1.0)´N¬O³Qshading¸?¹Lªº­?
                        Pure_AE_Block[i][j] = (int)((float)rCamInfo.rAEInfo.pu4AEBlock[i][j]/lsc_gain);  // could use PSO instead

                        Pure_New_AE_Block[i][j] = (Pure_AE_Block[i][j]*power_tbl[((abs_delta_idx)>>3)/rCamInfo.rAEInfo.u4AEFinerEVIdxBase] + (1<<19) ) >>20;
                        //Pure_New_AE_Block[i][j] = Pure_AE_Block[i][j]*pow((float)2,  (float)rCamInfo.rAEInfo.i4deltaIndex/8/rCamInfo.rAEInfo.u4AEFinerEVIdxBase/10);

                        Pure_New_Weight_AE_Block[i][j] = Pure_New_AE_Block[i][j]*rCamInfo.rAEInfo.pu4Weighting[i][j];

                        CWV1_Multiply_WeightSum -= Pure_New_Weight_AE_Block[i][j];

                        RA_Coeff += (float)Pure_New_Weight_AE_Block[i][j]*(LSC_GAIN_TABLE[i][j]-1.0)/(float)MAX_RA;
                    }
                }


                RA2 = (int)((float)CWV1_Multiply_WeightSum/RA_Coeff);
                */
                //memset(output_log, 0, sizeof(output_log));
                //strcpy(output_log, "pu4AEBlock\n");
                //print_ae_log_0(output_log);
                //for(i=0;i<AE_BLOCK_NO;i++)
                //{
                //    memset(output_log, 0, sizeof(output_log));
                //    strcpy(output_log, "%d %d %d %d %d\n");
                //    print_ae_log_5("%d %d %d %d %d\n", rCamInfo.rAEInfo.pu4AEBlock[i][0], rCamInfo.rAEInfo.pu4AEBlock[i][1], rCamInfo.rAEInfo.pu4AEBlock[i][2], rCamInfo.rAEInfo.pu4AEBlock[i][3], rCamInfo.rAEInfo.pu4AEBlock[i][4]);
                //}

                //memset(output_log, 0, sizeof(output_log));
                //strcpy(output_log, "Pure_AE_Block\n");
                //print_ae_log_0(output_log);
                //for(i=0;i<AE_BLOCK_NO;i++)
                //{
                //    memset(output_log, 0, sizeof(output_log));
                //    strcpy(output_log, "%d %d %d %d %d\n");
                //    print_ae_log_5("%d %d %d %d %d\n", Pure_AE_Block[i][0], Pure_AE_Block[i][1], Pure_AE_Block[i][2], Pure_AE_Block[i][3], Pure_AE_Block[i][4]);
                //}

                //memset(output_log, 0, sizeof(output_log));
                //strcpy(output_log, "Pure_New_AE_Block\n");
                //print_ae_log_0(output_log);
                //for(i=0;i<AE_BLOCK_NO;i++)
                //{
                //    memset(output_log, 0, sizeof(output_log));
                //    strcpy(output_log, "%d %d %d %d %d\n");
                //    print_ae_log_5("%d %d %d %d %d\n", Pure_New_AE_Block[i][0], Pure_New_AE_Block[i][1], Pure_New_AE_Block[i][2], Pure_New_AE_Block[i][3], Pure_New_AE_Block[i][4]);
                //}

                //memset(output_log, 0, sizeof(output_log));
                //strcpy(output_log, "Pure_New_Weight_AE_Block\n");
                //print_ae_log_0(output_log);
                //for(i=0;i<AE_BLOCK_NO;i++)
                //{
                //    memset(output_log, 0, sizeof(output_log));
                //    strcpy(output_log, "%d %d %d %d %d\n");
                //    print_ae_log_5("%d %d %d %d %d\n", Pure_New_Weight_AE_Block[i][0], Pure_New_Weight_AE_Block[i][1], Pure_New_Weight_AE_Block[i][2], Pure_New_Weight_AE_Block[i][3], Pure_New_Weight_AE_Block[i][4]);
                //}

                //memset(output_log, 0, sizeof(output_log));
                //strcpy(output_log, "CWV1_Multiply_WeightSum=%d\n");
                //print_ae_log_1(output_log, CWV1_Multiply_WeightSum);
                //
                //memset(output_log, 0, sizeof(output_log));
                //strcpy(output_log, "RA_Coeff=%d\n");
                //print_ae_log_1(output_log, RA_Coeff);

                //memset(output_log, 0, sizeof(output_log));
                //strcpy(output_log, "RA2=%d\n");
                //print_ae_log_1(output_log, RA2);

                SlowLSC = 0;
            }
            else
            {
                SlowLSC = 1;
            }


            if(NonLinearLSC)
            {
                if(rCamInfo.rAEInfo.i4deltaIndex>0)
                {
                    i4Rto = Dst_Ratio;
                }
                else
                {
                    i4Rto = Dst_Ratio;
                    //i4Rto = PreFrame1Ratio + (Dst_Ratio-PreFrame1Ratio)*rCamInfo.rAEInfo.i4EVRatio/100;
                }

            }
            else if(SlowLSC)
            {
                //if(Dst_Ratio>PreFrame1Ratio)
                //    i4Rto = PreFrame1Ratio+1;
                //else if(Dst_Ratio<PreFrame1Ratio)
                //    i4Rto = PreFrame1Ratio-1;
                //else
                 //   i4Rto = PreFrame1Ratio;

                i4Rto = Dst_Ratio;
            }
            else//(!rCamInfo.rAEInfo.bAEStable)
            {
                                                      //                                                  RA2        dst       RA1
                if(rCamInfo.rAEInfo.i4deltaIndex > 0) //image is brighting, iso upperARA lower    --------|----------|---------|------
                {
                    if(RA2 < Dst_Ratio) //RA2 is very safe, use dst result
                    {
                        //RA2 = PreFrame1Ratio + (Dst_Ratio-PreFrame1Ratio)*rCamInfo.rAEInfo.i4EVRatio/100 + 1; //check if use slower result
                        RA2 = Dst_Ratio;
                    }
                    else if(RA2>RA1) //RA2 is bigger than RA1, not reasonable, use the previous result+1
                    {
                        //if(Dst_Ratio>PreFrame1Ratio)
                        //   RA2 = PreFrame1Ratio+1;
                        //else if(Dst_Ratio<PreFrame1Ratio)
                        //    RA2 = PreFrame1Ratio-1;
                        //else
                        //    RA2 = PreFrame1Ratio;

                         RA2 = Dst_Ratio;
                    }
                    else
                    {
                        //RA2 = RA2;
                        RA2 = Dst_Ratio;
                    }
                }                                         //                                             RA1         dst      RA2
                else if(rCamInfo.rAEInfo.i4deltaIndex < 0) //image is darking, iso lowerARA uppder-------|-----------|--------|----
                {
                    if(RA2 > Dst_Ratio) //RA2 is very safe, use dst result
                    {
                        //RA2 = PreFrame1Ratio + (Dst_Ratio-PreFrame1Ratio)*rCamInfo.rAEInfo.i4EVRatio/100;  //check if use slower result
                        RA2 = Dst_Ratio;
                    }
                    else if(RA2<RA1) //RA2 is smaller than RA1, not reasonable, use the previous result+1
                    {
                        //if(Dst_Ratio>PreFrame1Ratio)
                        //    RA2 = PreFrame1Ratio+1;
                        //else if(Dst_Ratio<PreFrame1Ratio)
                        //    RA2 = PreFrame1Ratio-1;
                        //else
                        //    RA2 = PreFrame1Ratio;

                        RA2 = Dst_Ratio;
                    }
                    else
                    {
                        //RA2 = RA2;
                        RA2 = Dst_Ratio;
                    }
                }
                i4Rto = RA2;
            }


            PreFrame4Ratio = PreFrame3Ratio;
            PreFrame3Ratio = PreFrame2Ratio;
            PreFrame2Ratio = PreFrame1Ratio;
            PreFrame1Ratio = i4Rto;
            }

            //memset(output_log, 0, sizeof(output_log));
            //strcpy(output_log,"DstRatio=%d, ApplyRatio=%d, u4MgrCWValue=%d, u4AvgWValue=%d, OutSideAvgY=%d, TG=%d, i4deltaIndex=%d\n");
            //print_ae_log_7(output_log, ratioMapping(rCamInfo.rAEInfo.u4RealISOValue), i4Rto, rCamInfo.rAEInfo.u4MgrCWValue, rCamInfo.rAEInfo.u4AvgWValue, OutSideAvgY, rCamInfo.rAEInfo.TgCtrlRight*30, rCamInfo.rAEInfo.i4deltaIndex);
            MY_LOG_IF(fgLogEn,"RealIso=%d, DstRatio=%d, ApplyRatio=%d, u4MgrCWValue=%d, u4AvgWValue=%d, OutSideAvgY=%d, TG=%d, i4deltaIndex=%d\n",
               rCamInfo.rAEInfo.u4RealISOValue, ratioMapping(rCamInfo.rAEInfo.u4RealISOValue), i4Rto, rCamInfo.rAEInfo.u4MgrCWValue, rCamInfo.rAEInfo.u4AvgWValue, OutSideAvgY, rCamInfo.rAEInfo.TgCtrlRight*30, rCamInfo.rAEInfo.i4deltaIndex);
        }
    }

    MY_LOG_IF(isSuperNightMode,"[%s] before ratioMappingByEV ratio (%d)", __FUNCTION__, i4Rto);

    if(isSuperNightMode)
    {
        float ae_EV_Value = ((float)(rCamInfo.rAEInfo.i4AEComp)/(10.0));
        MY_LOG("[%s] ev in AE info is %.2f", __FUNCTION__, ae_EV_Value);
        i4Rto = ratioMappingByEV(i4Rto, ae_EV_Value, getSensorID());
    }

    MY_LOG_IF(isSuperNightMode,"[%s] after ratioMappingByEV ratio (%d)", __FUNCTION__, i4Rto);

    return i4Rto;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sample code for sub sensor customization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
EIndex_Shading_CCT_T
CTIspTuningCustom<ESensorDev_Sub>::
evaluate_Shading_CCT_index(RAWIspCamInfo const& rCamInfo) const
{
    MY_LOG("CTIspTuningCustom<ESensorDev_Main> %s()\n", __FUNCTION__);

    MINT32 i4CCT = rCamInfo.rAWBInfo.i4CCT;

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = rCamInfo.eIdx_Shading_CCT;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

    MINT32 const THL1 = 2500;//3257;
    MINT32 const THH1 = 2800;//3484;
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const THL2 = 4673;
    MINT32 const THH2 = 5155;
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

    switch  (rCamInfo.eIdx_Shading_CCT)
    {
    case eIDX_Shading_CCT_ALight:
        if  ( i4CCT < THH1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT <  TH2)
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_CWF:
        if  ( i4CCT < THL1 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THH2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    case eIDX_Shading_CCT_D65:
        if  ( i4CCT < TH1 )
        {
         eIdx_Shading_CCT_new = eIDX_Shading_CCT_ALight;
        }
        else if ( i4CCT < THL2 )
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_CWF;
        }
        else
        {
            eIdx_Shading_CCT_new = eIDX_Shading_CCT_D65;
        }
        break;
    }

    if  ( rCamInfo.eIdx_Shading_CCT != eIdx_Shading_CCT_new )
    {
        MY_LOG(
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , rCamInfo.eIdx_Shading_CCT, eIdx_Shading_CCT_new,i4CCT
        );
    }

    return  eIdx_Shading_CCT_new;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sample code for sub sensor customization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
EIndex_Shading_CCT_T
CTIspTuningCustomBySensor<ESensorDev_Main, IMX258_SENSOR_ID>::
evaluate_Shading_CCT_index(RAWIspCamInfo const& rCamInfo) const
{
    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = rCamInfo.eIdx_Shading_CCT;

    return  eIdx_Shading_CCT_new;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspTuningCustom::
remap_ISO_value(MUINT32 const u4Iso) const
{
    MUINT32 remapIso = u4Iso;

    //add your remap ISO code here

    //MY_LOG("[%s] ISO: in(%d), out(%d)", __FUNCTION__, u4Iso, remapIso);
    return remapIso;
}

MVOID*
IspTuningCustom::
get_feature_control(ESensorDev_T eSensorDev) const
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //main
        return &ispTopCtl;
    case ESensorDev_Sub: //sub
        return &ispTopCtl;
    case ESensorDev_MainSecond: //main2
        return &ispTopCtl;
    case ESensorDev_SubSecond: //main2
        return &ispTopCtl;
    default:
        return &ispTopCtl;
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID*
IspTuningCustom::
get_RawHDR_LCEGain_ConverParam() const
{
    return &RawHDR_LCEGain_ConverParam;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_fine_tune(ESensorDev_T const eSensorDev)
{
    switch (eSensorDev)
    {
    case ESensorDev_Main: //main
        return MFALSE;
    case ESensorDev_Sub: //sub
        return MFALSE;
    case ESensorDev_MainSecond: //main2
        return MFALSE;
    case ESensorDev_SubSecond: //main2
        return MFALSE;
    default:
        return MFALSE;

    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
fine_tune_ANR(RAWIspCamInfo const& rCamInfo,
                    NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo const& GmaExifInfo,
                    NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo const& LceExifInfo,
                    ISP_NVRAM_ANR_T const& rANR_U, ISP_NVRAM_ANR_T const& rANR_L,
                    ISP_NVRAM_ANR_T& rANR)
{
    (void)rCamInfo;
    (void)GmaExifInfo;
    (void)LceExifInfo;
    (void)rANR_U;
    (void)rANR_L;
    (void)rANR;
// Sensor Mode : rCamInfo.eSensorMode
// ISO Value : rCamInfo.rAEInfo.u4RealISOValue
// ISP Profile : rCamInfo.eIspProfile
// FD ratio : rCamInfo.FDRatio
// Bin enable : rCamInfo.BinInfo.fgBIN
// RRZ ratio (include bin) : rCamInfo.rCropRzInfo
// GMA info : GmaExifInfo
// LCE info : LceExifInfo
// Upper ANR : rANR_U, customANR_main[0][rCamInfo.eIdx_ISO_U], customANR_main[1][rCamInfo.eIdx_ISO_U]
// Lower ANR : rANR_L, customANR_main[0][rCamInfo.eIdx_ISO_L], customANR_main[1][rCamInfo.eIdx_ISO_L]
// After interpolation & Output : rANR


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
fine_tune_ANR2(RAWIspCamInfo const& rCamInfo,
                     NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo const& GmaExifInfo,
                     NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo const& LceExifInfo,
                     ISP_NVRAM_ANR2_T const& rANR2_U, ISP_NVRAM_ANR2_T const& rANR2_L,
                     ISP_NVRAM_ANR2_T& rANR2)
{
    (void)rCamInfo;
    (void)GmaExifInfo;
    (void)LceExifInfo;
    (void)rANR2_U;
    (void)rANR2_L;
    (void)rANR2;
// Sensor Mode : rCamInfo.eSensorMode
// ISO Value : rCamInfo.rAEInfo.u4RealISOValue
// ISP Profile : rCamInfo.eIspProfile
// FD ratio : rCamInfo.FDRatio
// Bin enable : rCamInfo.BinInfo.fgBIN
// RRZ ratio (include bin) : rCamInfo.rCropRzInfo
// GMA info : GmaExifInfo
// LCE info : LceExifInfo
// Upper ANR2 : rANR2_U, customANR2_main[0][rCamInfo.eIdx_ISO_U], customANR2_main[1][rCamInfo.eIdx_ISO_U]
// Lower ANR2 : rANR2_L, customANR2_main[0][rCamInfo.eIdx_ISO_L], customANR2_main[1][rCamInfo.eIdx_ISO_L]
// After interpolation & Output : rANR2


}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
fine_tune_EE(RAWIspCamInfo const& rCamInfo,
                  NSIspExifDebug::IspExifDebugInfo_T::IspGmaInfo const& GmaExifInfo,
                  NSIspExifDebug::IspExifDebugInfo_T::IspLceInfo const& LceExifInfo,
                  ISP_NVRAM_EE_T const& rEE_U, ISP_NVRAM_EE_T const& rEE_L,
                  ISP_NVRAM_EE_T& rEE)
{
    (void)rCamInfo;
    (void)GmaExifInfo;
    (void)LceExifInfo;
    (void)rEE_U;
    (void)rEE_L;
    (void)rEE;
// Sensor Mode : rCamInfo.eSensorMode
// ISO Value : rCamInfo.rAEInfo.u4RealISOValue
// ISP Profile : rCamInfo.eIspProfile
// FD ratio : rCamInfo.FDRatio
// Bin enable : rCamInfo.BinInfo.fgBIN
// RRZ ratio (include bin) : rCamInfo.rCropRzInfo
// GMA info : GmaExifInfo
// LCE info : LceExifInfo
// Upper EE : rEE_U, customEE_main[0][rCamInfo.eIdx_ISO_U], customEE_main[1][rCamInfo.eIdx_ISO_U]
// Lower EE : rEE_L, customEE_main[0][rCamInfo.eIdx_ISO_L], customEE_main[1][rCamInfo.eIdx_ISO_L]
// After interpolation & Output : rEE


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspTuningCustom::
adaptive_NR3D_setting(RAWIspCamInfo const& rCamInfo,
ISP_NR3D_SMOOTH_INFO_T const& rNR3D_SW_Params,
ISP_NVRAM_NR3D_T& rNR3D,
MINT32& GMVLevelTh,
ESensorDev_T eSensorDev)
{
#define CLIP_NR3D(a,b,c)        ( (a<b)?  b : ((a>c)? c: a) )

    (void)eSensorDev;
    MINT32 GMVX = rNR3D_SW_Params.M_Info.GMVX/256;
    MINT32 GMVY = rNR3D_SW_Params.M_Info.GMVY/256;
    MINT32 confX = rNR3D_SW_Params.M_Info.confX;
    MINT32 confY = rNR3D_SW_Params.M_Info.confY;
    MINT32 MAX_GMV = rNR3D_SW_Params.M_Info.MAX_GMV;
    MINT32 frameReset = rNR3D_SW_Params.M_Info.frameReset;
    //MINT32 GMV_Status = rNR3D_SW_Params.M_Info.GMV_Status;
    MINT32 GMV_level_th_def = rNR3D_SW_Params.C_Paras.GMV_level_th_def;

    MINT32 Device_ID = rNR3D_SW_Params.Device_ID;
    MBOOL  ISO_switch = MFALSE;//HW turned off
    MINT32 u4RealISO = rCamInfo.rAEInfo.u4RealISOValue;

    MINT32 GMV_sm_th = rNR3D_SW_Params.C_Paras.GMV_sm_th;//0.875*MAX_GMV 112/128=0.875, ranged:0~128
    MINT32 GMV_sm_th_up = rNR3D_SW_Params.C_Paras.GMV_sm_th_up;//minus point to GMV_level_th, ranged:0~255
    MINT32 GMV_sm_th_lo = rNR3D_SW_Params.C_Paras.GMV_sm_th_lo;//add point to GMV_level_th, ranged:0~255
    MINT32 GMV_lv_th_up = rNR3D_SW_Params.C_Paras.GMV_lv_th_up;//ranged:0~255
    MINT32 GMV_lv_th_lo = rNR3D_SW_Params.C_Paras.GMV_lv_th_lo;//ranged:0~255
    MINT32 GMV_min_str = rNR3D_SW_Params.C_Paras.GMV_min_str;//0.8*MAX_FLTSTR, 102/128=0.8, ranged:0~128
    MINT32 ISO_cutoff = rNR3D_SW_Params.M_Info.ISO_cutoff; // turned off when ISO < 400
    MINT32 ISO_upper = rNR3D_SW_Params.C_Paras.ISO_upper;
    //MINT32 Conf_th = rNR3D_SW_Params.C_Paras.Conf_th;
    MINT32 disableAdaptiveSetting = 0;

    MBOOL bAdaptiveNR3DDebug = ::property_get_int32("vendor.debug.nr3d.bm.enable", 0);

    if (frameReset)
    {
        if (GMVLevelTh != GMV_level_th_def)
        {
            MY_LOG_IF(bAdaptiveNR3DDebug>=1, "Device_ID(%d) Reset first GMVLevelTh to %d", Device_ID, GMV_level_th_def);
            GMVLevelTh = GMV_level_th_def;
        }
    }

    if (bAdaptiveNR3DDebug)
    {
        static MINT32 s_GMV_lv_th_def = 200;//frame 1 default value for "GMV_level_th", ranged:0~255

        MINT32 GMV_lv_th_def = ::property_get_int32("vendor.debug.nr3d.bm.GMV_lv_th_def", 200);

        if ((s_GMV_lv_th_def != GMV_lv_th_def) && (GMV_lv_th_def < 255))
        {
            s_GMV_lv_th_def = GMV_lv_th_def;
            GMVLevelTh = s_GMV_lv_th_def;
            MY_LOG_IF(bAdaptiveNR3DDebug>=1, "Reset s_GMV_lv_th_def and GMVLevelTh to %d", GMVLevelTh);
        }

        GMV_sm_th = ::property_get_int32("vendor.debug.nr3d.bm.GMV_sm_th", GMV_sm_th);
        GMV_sm_th_up = ::property_get_int32("vendor.debug.nr3d.bm.GMV_sm_th_up", GMV_sm_th_up);
        GMV_sm_th_lo = ::property_get_int32("vendor.debug.nr3d.bm.GMV_sm_th_lo", GMV_sm_th_lo);
        GMV_lv_th_up = ::property_get_int32("vendor.debug.nr3d.bm.GMV_lv_th_up", GMV_lv_th_up);
        GMV_lv_th_lo = ::property_get_int32("vendor.debug.nr3d.bm.GMV_lv_th_lo", GMV_lv_th_lo);
        GMV_min_str = ::property_get_int32("vendor.debug.nr3d.bm.GMV_min_str", GMV_min_str);
        ISO_cutoff = ::property_get_int32("vendor.debug.nr3d.bm.ISO_cutoff", ISO_cutoff);
        ISO_upper = ::property_get_int32("vendor.debug.nr3d.bm.ISO_upper", ISO_upper);
        disableAdaptiveSetting = ::property_get_int32("vendor.debug.nr3d.bm.disable_adapt", 0);

        MY_LOG_IF(bAdaptiveNR3DDebug>=2, "Set debug.nr3d.bm (%d, %d, %d, %d, %d, %d, %d, %d)",
              GMV_sm_th, GMV_sm_th_up, GMV_sm_th_lo, GMV_lv_th_up, GMV_lv_th_lo, GMV_min_str, ISO_cutoff, ISO_upper);
    }

    /////////////start of smooth GMV part
    //MAX GMV should be something like 32, 64, 128

    MUINT32 GMV_level_str = 128; //128 = 100%
    MUINT32 ISO_level_str = 128; //128 = 100%

    MINT32 i4GmvTooLrg=0;
    MINT32 i4GmvLowConf=0;

    if ( ( (abs(GMVX)) >= ( MAX_GMV * GMV_sm_th / 128) ) || ( (abs(GMVY)) >= ( MAX_GMV * GMV_sm_th /128 ) ) )
        i4GmvTooLrg = 1;

    if ( ( (abs(confX)) <= 10 ) || ( (abs(confY)) <= 10 ) )
        i4GmvLowConf = 1;


    if (i4GmvTooLrg==1)
        GMVLevelTh = GMVLevelTh - GMV_sm_th_up;
    else
        GMVLevelTh = GMVLevelTh + GMV_sm_th_lo;


    GMVLevelTh = CLIP_NR3D( GMVLevelTh, 0, 255);

    if( GMVLevelTh > GMV_lv_th_up )
    {
        GMV_level_str = 128;
    }
    else
    {
        if (0 != (GMV_lv_th_up - GMV_lv_th_lo))
        {
            GMV_level_str = GMV_min_str + ( ( (128 - GMV_min_str) *  CLIP_NR3D( GMVLevelTh - GMV_lv_th_lo, 0, GMV_lv_th_up - GMV_lv_th_lo ) +( GMV_lv_th_up - GMV_lv_th_lo )/2 ) / ( GMV_lv_th_up - GMV_lv_th_lo ) );
        }
        else
        {
            MY_LOG_IF(bAdaptiveNR3DDebug>=1, "0 = (GMV_lv_th_up - GMV_lv_th_lo)");
        }
    }

    /////////////end of smooth GMV part




    /////////////start of smooth ISO part

    if( u4RealISO <= ISO_cutoff )
    {
        ISO_switch = 0;//HW turned off
    }
    else if( u4RealISO > ISO_cutoff && u4RealISO < ISO_upper )
    {
        if (0 != (ISO_upper - ISO_cutoff))
        {
            ISO_level_str = 0 +  ( ( (128 - 0) * CLIP_NR3D( u4RealISO - ISO_cutoff, 0, ISO_upper - ISO_cutoff ) + ( ISO_upper - ISO_cutoff )/2 ) / ( ISO_upper - ISO_cutoff ) );
        }
        else
        {
            MY_LOG_IF(bAdaptiveNR3DDebug>=1, "0 = (ISO_upper - ISO_cutoff)");
        }
        ISO_switch = 1;
    }
    else
    {
        ISO_switch = 1;
        ISO_level_str = 128;
    }
    /////////////end of smooth ISO part

    /////////////start of SMOOTH GMV/ISO part
    ///////////////start of 20170316
    MINT32 enableAdaptiveISO = 0;
    if (bAdaptiveNR3DDebug)
    {
        enableAdaptiveISO = ::property_get_int32("vendor.debug.nr3d.bm.enable_ISOadpt", 0);
    }

    MINT32 disableGMVConfRefine = 0;

    if (bAdaptiveNR3DDebug)
    {
        disableGMVConfRefine = ::property_get_int32("vendor.debug.nr3d.bm.disable_GMVRef", 0);
     }

    // === 2016/03/16: gmv adjustment by conf: start ==
    MINT32 i4GmvConfFallback=0;
    MINT32 adjustGMVX = GMVX;
    MINT32 adjustGMVY = GMVY;

    if (disableGMVConfRefine)
    {
        i4GmvConfFallback=0;
    }
    else
    {
        if(NR3DCustom::adjust_3dnr_gmv_by_conf(1, confX, confY, adjustGMVX, adjustGMVY))
        {
            // no need adjust GMVX/Y to adjustGMVX/Y here because algo won't use adjusted GMVX/Y later
            i4GmvConfFallback=1;
            MY_LOG_IF(bAdaptiveNR3DDebug>=1, "GMV adjusted: (confX,confY)=(%d,%d), gmvX(%d->%d), gmvY(%d->%d)",
                confX, confY, GMVX, adjustGMVX, GMVY, adjustGMVY);
        }
    }
    // === 2016/03/16: gmv adjustment by conf: end ==

    MINT32 GMV_ISO_RATIO=(GMV_level_str); //(/128)
    MINT32 GMV_ISO_BSE=128; //(/128)
    MINT32 GMV_ISO_RND=64;   //(/128)
    MINT32 NR3D_FLT_STR_MAX_before = rNR3D.tnr_flt_config.bits.NR3D_FLT_STR_MAX;
    MINT32 NR3D_FLT_STR_MAX_after = ( (NR3D_FLT_STR_MAX_before * GMV_ISO_RATIO) + GMV_ISO_RND ) / GMV_ISO_BSE;

    if(enableAdaptiveISO == 1)
    {
        GMV_ISO_RATIO=(GMV_level_str*ISO_level_str); //(/128/128)
        GMV_ISO_BSE=128*128; //(/128/128)
        GMV_ISO_RND=128*128/2;   //(/128/128)
        NR3D_FLT_STR_MAX_before = rNR3D.tnr_flt_config.bits.NR3D_FLT_STR_MAX;
        NR3D_FLT_STR_MAX_after = ( (NR3D_FLT_STR_MAX_before * GMV_ISO_RATIO) + GMV_ISO_RND ) / GMV_ISO_BSE;
    }


    //if (0 == disableAdaptiveSetting)
    if (0 == disableAdaptiveSetting && i4GmvConfFallback==0)
    {
        rNR3D.tnr_flt_config.bits.NR3D_FLT_STR_MAX = NR3D_FLT_STR_MAX_after;
    }
    ///////////////end of 20170316

    /////////////end of SMOOTH GMV/ISO part

    ////////////DUMP
    MY_LOG_IF(bAdaptiveNR3DDebug>=1, "Device_ID(%d), MAX_GMV:%d, (GMVX:%d,GMVY:%d), (confX:%d,confY:%d), m_GMV_level_th:%d, GMV_level_str:%d, u4RealISO:%d, ISO_level_str:%d, ISO_switch:%d",
        Device_ID, MAX_GMV, GMVX, GMVY, confX, confY, GMVLevelTh, GMV_level_str, u4RealISO, ISO_level_str, ISO_switch);

    MY_LOG_IF(bAdaptiveNR3DDebug>=1, "Device_ID(%d), MAX_FLTSTRbefore=%d, MAX_FLTSTRafter=%d, disableAdaptiveSetting(%d)\n",
        Device_ID, NR3D_FLT_STR_MAX_before, NR3D_FLT_STR_MAX_after, disableAdaptiveSetting);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspTuningCustom::
is_to_invoke_remap_ISP_ISO_with_LV(RAWIspCamInfo const& rCamInfo)
{
    (void)rCamInfo;
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspTuningCustom::
remap_ISP_ISO_with_LV(ISP_NVRAM_ISO_INTERVAL_STRUCT const& rISO_ENV, MUINT32 const LV, MUINT32 const realISO, MBOOL enable) //(float isoIndex, int lvValue, bool enable)
{

    MBOOL bDebug_LogEn = ::property_get_int32("vendor.debug.custom.remap", 0);

    MUINT32 i = 0;
    MUINT32 lvIndex = 0;
    MUINT32 upperIsoIndex = 0;
    MUINT32 lowerIsoIndex = 0;
    float isoRefineIndex = 0;
    float isoIndex = 0;
    MUINT32 isoRefine = 0;

    if(enable == 0 || rISO_ENV.u2Length != 20 || realISO > (const MUINT32)rISO_ENV.IDX_Partition[rISO_ENV.u2Length-1]   || realISO < (const MUINT32)rISO_ENV.IDX_Partition[0]   )
      return realISO;

    float LV_LUT[20][20] = {

      //LV  0      1      2     3     4     5     6     7    8   9     10    11    12    13    14    15    16    17    18    19
         { 0.8 , 0.7  ,  0.6 , 0.5 , 0.4 , 0.3 , 0.2 , 0.1 , 0 , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0 } , //iso index 0
         { 1.8 , 1.7  ,  1.6 , 1.5 , 1.4 , 1.3 , 1.2 , 1.1 , 1 , 1   , 1   , 1   , 0.9 , 0.8 , 0.7 , 0.6 , 0.5 , 0.4 , 0.3 , 0.2 } , //iso index 1
         { 2.8 , 2.7  ,  2.6 , 2.5 , 2.4 , 2.3 , 2.2 , 2.1 , 2 , 2   , 2   , 2   , 1.9 , 1.8 , 1.7 , 1.6 , 1.5 , 1.4 , 1.3 , 1.2 } , //iso index 2
         { 3.8 , 3.7  ,  3.6 , 3.5 , 3.4 , 3.3 , 3.2 , 3.1 , 3 , 3   , 3   , 3   , 2.9 , 2.8 , 2.7 , 2.6 , 2.5 , 2.4 , 2.3 , 2.2 } , //iso index 3
         { 4.8 , 4.7  ,  4.6 , 4.5 , 4.4 , 4.3 , 4.2 , 4.1 , 4 , 4   , 4   , 4   , 3.9 , 3.8 , 3.7 , 3.6 , 3.5 , 3.4 , 3.3 , 3.2 } , //iso index 4
         { 5.8 , 5.7  ,  5.6 , 5.5 , 5.4 , 5.3 , 5.2 , 5.1 , 5 , 5   , 5   , 5   , 4.9 , 4.8 , 4.7 , 4.6 , 4.5 , 4.4 , 4.3 , 4.2 } , //iso index 5
         { 6.8 , 6.7  ,  6.6 , 6.5 , 6.4 , 6.3 , 6.2 , 6.1 , 6 , 6   , 6   , 6   , 5.9 , 5.8 , 5.7 , 5.6 , 5.5 , 5.4 , 5.3 , 5.2 } , //iso index 6
         { 7.8 , 7.7  ,  7.6 , 7.5 , 7.4 , 7.3 , 7.2 , 7.1 , 7 , 7   , 7   , 7   , 6.9 , 6.8 , 6.7 , 6.6 , 6.5 , 6.4 , 6.3 , 6.2 } , //iso index 7
         { 8.8 , 8.7  ,  8.6 , 8.5 , 8.4 , 8.3 , 8.2 , 8.1 , 8 , 8   , 8   , 8   , 7.9 , 7.8 , 7.7 , 7.6 , 7.5 , 7.4 , 7.3 , 7.2 } , //iso index 8
         { 9.8 , 9.7  ,  9.6 , 9.5 , 9.4 , 9.3 , 9.2 , 9.1 , 9 , 9   , 9   , 9   , 8.9 , 8.8 , 8.7 , 8.6 , 8.5 , 8.4 , 8.3 , 8.2 } , //iso index 9
         { 10.8, 10.7 ,  10.6, 10.5, 10.4, 10.3, 10.2, 10.1, 10, 10  , 10  , 10  , 9.9 , 9.8 , 9.7 , 9.6 , 9.5 , 9.4 , 9.3 , 9.2 } , //iso index 10
         { 11.8, 11.7 ,  11.6, 11.5, 11.4, 11.3, 11.2, 11.1, 11, 11  , 11  , 11  , 10.9, 10.8, 10.7, 10.6, 10.5, 10.4, 10.3, 10.2} , //iso index 11
         { 12.8, 12.7 ,  12.6, 12.5, 12.4, 12.3, 12.2, 12.1, 12, 12  , 12  , 12  , 11.9, 11.8, 11.7, 11.6, 11.5, 11.4, 11.3, 11.2} , //iso index 12
         { 13.8, 13.7 ,  13.6, 13.5, 13.4, 13.3, 13.2, 13.1, 13, 13  , 13  , 13  , 12.9, 12.8, 12.7, 12.6, 12.5, 12.4, 12.3, 12.2} , //iso index 13
         { 14.8, 14.7 ,  14.6, 14.5, 14.4, 14.3, 14.2, 14.1, 14, 14  , 14  , 14  , 13.9, 13.8, 13.7, 13.6, 13.5, 13.4, 13.3, 13.2} , //iso index 14
         { 15.8, 15.7 ,  15.6, 15.5, 15.4, 15.3, 15.2, 15.1, 15, 15  , 15  , 15  , 14.9, 14.8, 14.7, 14.6, 14.5, 14.4, 14.3, 14.2} , //iso index 15
         { 16.8, 16.7 ,  16.6, 16.5, 16.4, 16.3, 16.2, 16.1, 16, 16  , 16  , 16  , 15.9, 15.8, 15.7, 15.6, 15.5, 15.4, 15.3, 15.2} , //iso index 16
         { 17.8, 17.7 ,  17.6, 17.5, 17.4, 17.3, 17.2, 17.1, 17, 17  , 17  , 17  , 16.9, 16.8, 16.7, 16.6, 16.5, 16.4, 16.3, 16.2} , //iso index 17
         { 18.8, 18.7 ,  18.6, 18.5, 18.4, 18.3, 18.2, 18.1, 18, 18  , 18  , 18  , 17.9, 17.8, 17.7, 17.6, 17.5, 17.4, 17.3, 17.2} , //iso index 18
         { 19  , 19   ,  19  , 19  , 19  , 19  , 19  , 19  , 19, 19  , 19  , 19  , 18.9, 18.8, 18.7, 18.6, 18.5, 18.4, 18.3, 18.2} , //iso index 19


    };

    //compute iso index
    if((const MUINT32)rISO_ENV.IDX_Partition[0] >= realISO)
    {
          lowerIsoIndex = 0;
          upperIsoIndex = 0;
          isoIndex = 0;
    }
    else if ((const MUINT32)rISO_ENV.IDX_Partition[19] < realISO)
    {
          lowerIsoIndex = 19;
          upperIsoIndex = 19;
          isoIndex = 19;
    }
    else
    {
        for(i = 1; i <rISO_ENV.u2Length; ++i )
        {
            if((const MUINT32)rISO_ENV.IDX_Partition[i] == realISO)
            {
              lowerIsoIndex = i;
              upperIsoIndex = i;
              isoIndex = i;
              break;
            }
            else if(((const MUINT32)rISO_ENV.IDX_Partition[i] > realISO) && ((const MUINT32)rISO_ENV.IDX_Partition[i-1] < realISO))
            {
              lowerIsoIndex = i-1;
              upperIsoIndex = i;

              if(rISO_ENV.IDX_Partition[upperIsoIndex] == rISO_ENV.IDX_Partition[lowerIsoIndex] )
                isoIndex = lowerIsoIndex;
              else
                isoIndex = lowerIsoIndex + (float)(realISO - rISO_ENV.IDX_Partition[lowerIsoIndex])/(rISO_ENV.IDX_Partition[upperIsoIndex] -rISO_ENV.IDX_Partition[lowerIsoIndex] );
              break;
            }
        }
    }

    MY_LOG_IF(bDebug_LogEn,"isoIndex = %d", (MINT32)isoIndex);

    upperIsoIndex = CEIL(isoIndex);
    lowerIsoIndex = FLOOR(isoIndex);

    upperIsoIndex = (upperIsoIndex > 19)? 19: upperIsoIndex;
    //upperIsoIndex = (upperIsoIndex < 0)? 0: upperIsoIndex;

    lowerIsoIndex = (lowerIsoIndex > 19)? 19: lowerIsoIndex;
    //lowerIsoIndex = (lowerIsoIndex < 0)? 0: lowerIsoIndex;

    MY_LOG_IF(bDebug_LogEn,"upperIsoIndex = %d, lowerIsoIndex = %d", upperIsoIndex, lowerIsoIndex);

    lvIndex = (LV+ 5)/10;
    lvIndex = (lvIndex > 19)? 19: lvIndex;
    //lvIndex = (lvIndex <0 )? 0: lvIndex;

    MY_LOG_IF(bDebug_LogEn,"lvIndex = %d", lvIndex);

    if (lowerIsoIndex == upperIsoIndex)
      isoRefineIndex =  LV_LUT[(int)lowerIsoIndex][lvIndex];
    else
      isoRefineIndex = (LV_LUT[(int)lowerIsoIndex][lvIndex]*(upperIsoIndex - isoIndex) + LV_LUT[(int)upperIsoIndex][lvIndex] * (isoIndex-lowerIsoIndex)) / (upperIsoIndex - lowerIsoIndex);

    upperIsoIndex = CEIL(isoRefineIndex);
    lowerIsoIndex = FLOOR(isoRefineIndex);

    upperIsoIndex = (upperIsoIndex > 19)? 19: upperIsoIndex;
    //upperIsoIndex = (upperIsoIndex < 0)? 0: upperIsoIndex;

    lowerIsoIndex = (lowerIsoIndex > 19)? 19: lowerIsoIndex;
    //lowerIsoIndex = (lowerIsoIndex < 0)? 0: lowerIsoIndex;

    isoRefine =  rISO_ENV.IDX_Partition[lowerIsoIndex] +  (isoRefineIndex - lowerIsoIndex)*(rISO_ENV.IDX_Partition[upperIsoIndex] -rISO_ENV.IDX_Partition[lowerIsoIndex] ) ;

    MY_LOG_IF(bDebug_LogEn,"isoRefineIndex = %d, isoRefine = %d", (MINT32)isoRefineIndex, isoRefine);

    return isoRefine;
}

MUINT32
IspTuningCustom::
get_HdrDbsNewISO(RAWIspCamInfo const& rCamInfo)
{
    MUINT32 u4LE2SEGainRtoX100 = 100;
    MUINT32 u4LESensorISO = rCamInfo.rAEInfo.u4RealISOValue;
    MUINT32 u4SESensorISO = rCamInfo.rAEInfo.u4RealISOValue;

   if (rCamInfo.rAEInfo.rHdrAEInfo.rHdrEVInfo.i4SEAfeGain != 0){
              u4LE2SEGainRtoX100 = (rCamInfo.rAEInfo.rHdrAEInfo.rHdrEVInfo.i4LEAfeGain * 100) / ( rCamInfo.rAEInfo.rHdrAEInfo.rHdrEVInfo.i4SEAfeGain);
   }

   if (rCamInfo.rAEInfo.rHdrAEInfo.rHdrEVInfo.i4LEIspGain != 0){
              u4LESensorISO = (rCamInfo.rAEInfo.u4RealISOValue * 1024)/ rCamInfo.rAEInfo.rHdrAEInfo.rHdrEVInfo.i4LEIspGain;
   }

   if (u4LE2SEGainRtoX100 != 0){
              u4SESensorISO = (u4LESensorISO * 100)/ u4LE2SEGainRtoX100;
   }

   MUINT32 Alpha = 0; // 256 : means always use LEGain, 0 : means always use SEGain, default = 0

   MUINT32 NewISO = ((Alpha * u4LESensorISO + (256 - Alpha)* u4SESensorISO + 128)) >> 8;

   return NewISO;
}

MINT32
IspTuningCustom::
map_AppName_to_MappingInfo(const char* pAppname){

    char AppName[512] = {'\0'};
    property_get("vendor.debug.appcontrol.name", AppName, "MTKCam"); //replace property here

    if (strcmp (AppName,"MTKCam") != 0)  //use adb command value if not equal to default value, else use parameter pAppname
        snprintf(AppName, sizeof(AppName), "%s", pAppname);

    if(strstr(AppName, "Facebook")){
        return EApp_Facebook;
    } else if (strstr(AppName, "Line")){
        return EApp_Line;
    } else if (strstr(AppName, "QQ")){
        return EApp_QQ;
    } else if (strstr(AppName, "Wechat")){
        return EApp_Wechat;
    } else if (strstr(AppName, "Skype")){
        return EApp_Skype;
    } else if (strstr(AppName, "Normal")){
        return EApp_Normal;
    } else if (strstr(AppName, "Professional")){
        return EApp_Professional;
    } else if (strstr(AppName, "FaceBeauty")){
        return EApp_FaceBeauty;
    } else if (strstr(AppName, "HDR")){
        return EApp_HDR;
    } else if (strstr(AppName, "Panorama")){
        return EApp_Panorama;
    } else if (strstr(AppName, "Video")){
        return EApp_Video;
    } else if (strstr(AppName, "3rd_party")){
        return EApp_3rd_party;
    }
    else{
        return EApp_MTKCam;
    }
/*
    typedef enum
    {
        EApp_MTKCam,
        EApp_Facebook,
        EApp_Line,
        EApp_QQ,
        EApp_Wechat,
        EApp_Skype,
        EApp_Normal,
        EApp_Professional,
        EApp_FaceBeauty,
        EApp_HDR,
        EApp_Panorama,
        EApp_Video,
        EApp_3rd_party,
        EApp_NUM,
    } EApp_T;
*/
}

MINT32
IspTuningCustom::
map_FlashId_to_MappingInfo(int flashId){

        return EDriverIC_DriverIC_00;
/*
    typedef enum
    {
        EDriverIC_DriverIC_00,
        EDriverIC_DriverIC_01,
        EDriverIC_NUM,
    } EDriverIC_T;
*/
}

MINT32
IspTuningCustom::
map_P2inSize_to_MappingInfo(RAWIspCamInfo const& rCamInfo){
    if (rCamInfo.fgRPGEnable)
        return EP2Size_RRZO_00;
    else
        return EP2Size_IMGO;
/*
   input width: rCamInfo.rCropRz.i4RzWidth
   input height: rCamInfo.rCropRz.i4RzHeight
*/

/*
    typedef enum
    {
        EP2Size_IMGO,
        EP2Size_RRZO_00,
        EP2Size_RRZO_01,
        EP2Size_RRZO_02,
        EP2Size_1080P,
        EP2Size_720P,
        EP2Size_480P,
        EP2Size_NUM,
    } EP2Size_T;
*/
}

