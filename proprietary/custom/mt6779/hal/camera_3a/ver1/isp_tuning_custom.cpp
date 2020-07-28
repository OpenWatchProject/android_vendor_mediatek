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
#include <isp_mgr.h>
#include <stdlib.h>                     // For atoi()
#include <stdio.h>
#include <cutils/properties.h>  // For property_get().
#include "camera_custom_3dnr.h"
//#include <MTKGma.h>
//#include <MTKLce.h>
#include "n3d_sync2a_tuning_param.h"
#include <math.h>

#define CEIL(a)   ( (a-(int)a) == 0 ? (int)a : (int)(a+1))

#define FLOOR(a)  ( (int)a)

#ifndef LOG2
#define LOG2(x)       ((MDOUBLE)log(x)/log(2))
#endif


using namespace NSIspTuning;


#define TUNING_FOR_AIS  0
#define AIS_INTERPOLATION   0

static float AIS_NORMAL_CFA_RATIO[4] = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively
static float AIS_NORMAL_YNR_RATIO[4] = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively
static float AIS_NORMAL_CNR_RATIO[4] = {0.20f, 1.00f, 0.20f, 0.20f}; //0=ais, 1=normal, for stage1-4 respectively
static float AIS_NORMAL_EE_RATIO[4]  = {0.10f, 1.00f, 0.50f, 0.50f}; //0=ais, 1=normal, for stage1-4 respectively

#define IS_AIS          (rCamInfo.rAEInfo.u4OrgRealISOValue != rCamInfo.rAEInfo.u4RealISOValue)

#define FINE_TUNE_SUPPORT_NUM 2

static ISP_FEATURE_TOP_CTL ispTopCtl[EIspProfile_NUM] =
{
    //0:disable, 1:enable

    //EIsProfile_Preview 00
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(1) | F_NDG_EN(1) ),

    //EIspProfile_Video 01
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(1) | F_NDG_EN(1) ),

    //EIspProfile_Capture 02
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(1) | F_NDG_EN(1) ),

    //EIspProfile_Capture_DCE 03
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(0)   |F_DM_EN(0)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(0)    |F_GGM_EN(0)    |F_YNR_EN(0)   |F_LCE_EN(0)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(0)     |F_CNR_EN(0)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Capture_DSDN 04
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(1)   |F_DM_EN(0)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(0)    |F_GGM_EN(0)    |F_YNR_EN(1)   |F_LCE_EN(0)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(0)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_MFNR_Before_Blend 05
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(0)     |F_CNR_EN(1)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(1) | F_NDG_EN(1) ),

    //EIspProfile_MFNR_Single 06
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(1)   |F_DM_EN(0)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(0)    |F_GGM_EN(0)    |F_YNR_EN(1)   |F_LCE_EN(0)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(0)     |F_CNR_EN(1)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_MFNR_MFB 07
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(0)   |F_DM_EN(0)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(0)    |F_GGM_EN(0)    |F_YNR_EN(0)   |F_LCE_EN(0)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(0)     |F_CNR_EN(0)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(1)),

    //EIspProfile_MFNR_After_Blend 08
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(1)   |F_DM_EN(0)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(0)    |F_GGM_EN(0)    |F_YNR_EN(1)   |F_LCE_EN(0)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(1)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Preview 09
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Preview_toW 10
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Video 11
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Video_toW 12
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Capture 13
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Capture_Full 14
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Capture_Depth 15
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_N3D_Capture_Depth_toW 16
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Bokeh 17
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(0)   |F_DM_EN(0)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(0)    |F_GGM_EN(0)    |F_YNR_EN(0)   |F_LCE_EN(0)   |F_DCE_EN(0)    |F_LSC_EN(0)
    |F_EE_EN(0)     |F_CNR_EN(0)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(1)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Preview 18
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_mHDR_Preview 19
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Video 20
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_mHDR_Video 21
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Preview_VSS 22
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_mHDR_Preview_VSS 23
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Video_VSS 24
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_mHDR_Video_VSS 25
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Capture 26
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Capture_DCE 27
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Capture_DSDN 28
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_mHDR_Capture 29
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_mHDR_Capture_DCE 30
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_mHDR_Capture_DSDN 31
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Capture_MFNR_Before_Blend 32
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Capture_MFNR_Single 33
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Capture_MFNR_MFB 34
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_zHDR_Capture_MFNR_After_Blend 35
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_zHDR_Preview 36
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_mHDR_Preview 37
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_zHDR_Video 38
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_mHDR_Video 39
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(0)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(0)     |F_CNR_EN(0)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(0) ),

    //EIspProfile_Auto_zHDR_Preview_VSS 40
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_mHDR_Preview_VSS 41
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_zHDR_Video_VSS 42
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_mHDR_Video_VSS 43
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_zHDR_Capture 44
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_zHDR_Capture_DCE 45
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_zHDR_Capture_DSDN 46
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_Auto_mHDR_Capture 47
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(0)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) ),

    //EIspProfile_Auto_mHDR_Capture_DCE 48
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(0)   |F_DM_EN(0)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(0)    |F_GGM_EN(0)    |F_YNR_EN(0)   |F_LCE_EN(0)   |F_DCE_EN(1)    |F_LSC_EN(0)
    |F_EE_EN(0)     |F_CNR_EN(0)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) ),

    //EIspProfile_Auto_mHDR_Capture_DSDN 49
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_EIS_Preview 50
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_EIS_Video 51
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_AINR_Main 52
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(1) ),

    //EIspProfile_AINR_MainYUV 53
    (F_OBC_EN(0)    |F_BPC_EN(0)    |F_CT_EN(0)    |F_PDC_EN(0)   |F_DGN_EN(0)    |F_WB_EN(0)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(0)   |F_DM_EN(1)    |F_LDNR_EN(0)   |F_FLC_EN(0)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(0)   |F_LCE_EN(0)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(0)     |F_CNR_EN(0)    |F_CCR_EN(0)   |F_ABF_EN(0)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_AINR_Single 54
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(0)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(0)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(1) | F_NDG_EN(1) ),

    //EIspProfile_P1_YUV_HighRes 55
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_P1_YUV_FD 56
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_P1_YUV_Depth 57
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_SWHDR_RAW_DOMAIN 58
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_SWHDR_YUV_DOMAIN 59
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),

    //EIspProfile_YUV_Reprocess 60
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(1)    |F_LTM_EN(1)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(1)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) ),
    //EIspProfile_3rd_Party_Processed_Raw_Reprocess 61
    (F_OBC_EN(1)    |F_BPC_EN(1)    |F_CT_EN(1)    |F_PDC_EN(1)   |F_DGN_EN(1)    |F_WB_EN(1)
    |F_HLR_EN(0)    |F_LTM_EN(0)    |F_SLK_EN(1)   |F_DM_EN(1)    |F_LDNR_EN(1)   |F_FLC_EN(1)
    |F_CCM_EN(1)    |F_GGM_EN(1)    |F_YNR_EN(1)   |F_LCE_EN(0)   |F_DCE_EN(1)    |F_LSC_EN(1)
    |F_EE_EN(1)     |F_CNR_EN(1)    |F_CCR_EN(1)   |F_ABF_EN(1)   |F_BOK_EN(0)    |F_COLOR_EN(1)
    |F_MIX_EN(0)    |F_MFB_EN(0)    |F_YNR_LINK_EN(0) | F_NDG_EN(1) )
};

MVOID*
IspTuningCustom::
get_feature_control(MUINT32 eSensorDev) const
{
    switch (eSensorDev)
    {
    case ESensorDev_Main:       //main
        return &ispTopCtl;
    case ESensorDev_Sub:        //sub
        return &ispTopCtl;
    case ESensorDev_MainSecond: //main2
        return &ispTopCtl;
    case ESensorDev_SubSecond:  //sub2
        return &ispTopCtl;
    case ESensorDev_MainThird: //main3
        return &ispTopCtl;
    default:
        return &ispTopCtl;
    }

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


MVOID
IspTuningCustom::
refine_CamInfo(RAWIspCamInfo& rCamInfo)
{
    (void)rCamInfo;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_OBC(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_OBC_T& rOBC)
{
    (void)rCamInfo;
    (void)rOBC;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_BPC(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_BPC_BPC_T& rBPC)
{
    (void)rCamInfo;
    (void)rBPC;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_CT(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_BPC_CT_T& rCT)
{
    (void)rCamInfo;
    (void)rCT;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_PDC(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_BPC_PDC_T& rPDC)
{
    (void)rCamInfo;
    (void)rPDC;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_DM(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DM_T& rDM)
{
    (void)rCamInfo;
    (void)rDM;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_FLC(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_FLC_T& rFLC)
{
    (void)rCamInfo;
    (void)rFLC;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_CCM(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CCM_T& rCCM)
{
    switch (SubModuleIndex)
    {
        case ISP_MGR_CCM::ECCM_D1:
            if(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Depth_toW)
            {
                if( !m_CCM_DualPara_valid ){
                    const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
                    ::memcpy(m_CCM_DualPara, pSyncAEInitInfo->RGB2YCoef_main, sizeof(MUINT32)*3);
                    m_CCM_DualPara_valid = MTRUE;
                }
                RGB2WMatrix( m_CCM_DualPara, rCamInfo.rAWBInfo.rCurrentAWBGain, rCCM);
            }
            break;
        case ISP_MGR_CCM::ECCM_D2:
            {
                if(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview_toW )
                {
                    if( !m_CCM_DualPara_valid ){
                        const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
                        ::memcpy(m_CCM_DualPara, pSyncAEInitInfo->RGB2YCoef_main, sizeof(MUINT32)*3);
                        m_CCM_DualPara_valid = MTRUE;
                    }
                    RGB2WMatrix( m_CCM_DualPara, rCamInfo.rAWBInfo.rCurrentAWBGain, rCCM);
                }

                if(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Preview)
                {
                    ISP_NVRAM_CCM_T rCCMIn = {0};
                    ::memcpy(&rCCMIn, &rCCM, sizeof(ISP_NVRAM_CCM_T));
                    RGB2YLinear(rCCMIn, rCCM);
                }
            }
            break;
        default:
            break;
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
    W.cnv_1.bits.CCM_CNV_00 = iL[0];
    W.cnv_1.bits.CCM_CNV_01 = iL[1];
    W.cnv_2.bits.CCM_CNV_02 = iL[2];
    W.cnv_3.bits.CCM_CNV_10 = iL[0];
    W.cnv_3.bits.CCM_CNV_11 = iL[1];
    W.cnv_4.bits.CCM_CNV_12 = iL[2];
    W.cnv_5.bits.CCM_CNV_20 = iL[0];
    W.cnv_5.bits.CCM_CNV_21 = iL[1];
    W.cnv_6.bits.CCM_CNV_22 = iL[2];
}


MVOID
IspTuningCustom::
RGB2YLinear(const ISP_NVRAM_CCM_T& inW, ISP_NVRAM_CCM_T& outW)
{
    const int accuracy = 11;

    float fL[3];
    int iL[3];

    // transform to signed value
    int transW[9];
    transW[0] = inW.cnv_1.bits.CCM_CNV_00 > 4095 ? (int)inW.cnv_1.bits.CCM_CNV_00 - 8192 : inW.cnv_1.bits.CCM_CNV_00;
    transW[1] = inW.cnv_1.bits.CCM_CNV_01 > 4095 ? (int)inW.cnv_1.bits.CCM_CNV_01 - 8192 : inW.cnv_1.bits.CCM_CNV_01;
    transW[2] = inW.cnv_2.bits.CCM_CNV_02 > 4095 ? (int)inW.cnv_2.bits.CCM_CNV_02 - 8192 : inW.cnv_2.bits.CCM_CNV_02;
    transW[3] = inW.cnv_3.bits.CCM_CNV_10 > 4095 ? (int)inW.cnv_3.bits.CCM_CNV_10 - 8192 : inW.cnv_3.bits.CCM_CNV_10;
    transW[4] = inW.cnv_3.bits.CCM_CNV_11 > 4095 ? (int)inW.cnv_3.bits.CCM_CNV_11 - 8192 : inW.cnv_3.bits.CCM_CNV_11;
    transW[5] = inW.cnv_4.bits.CCM_CNV_12 > 4095 ? (int)inW.cnv_4.bits.CCM_CNV_12 - 8192 : inW.cnv_4.bits.CCM_CNV_12;
    transW[6] = inW.cnv_5.bits.CCM_CNV_20 > 4095 ? (int)inW.cnv_5.bits.CCM_CNV_20 - 8192 : inW.cnv_5.bits.CCM_CNV_20;
    transW[7] = inW.cnv_5.bits.CCM_CNV_21 > 4095 ? (int)inW.cnv_5.bits.CCM_CNV_21 - 8192 : inW.cnv_5.bits.CCM_CNV_21;
    transW[8] = inW.cnv_6.bits.CCM_CNV_22 > 4095 ? (int)inW.cnv_6.bits.CCM_CNV_22 - 8192 : inW.cnv_6.bits.CCM_CNV_22;

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

    outW.cnv_1.bits.CCM_CNV_00 = iL[0];
    outW.cnv_1.bits.CCM_CNV_01 = iL[1];
    outW.cnv_2.bits.CCM_CNV_02 = iL[2];
    outW.cnv_3.bits.CCM_CNV_10 = iL[0];
    outW.cnv_3.bits.CCM_CNV_11 = iL[1];
    outW.cnv_4.bits.CCM_CNV_12 = iL[2];
    outW.cnv_5.bits.CCM_CNV_20 = iL[0];
    outW.cnv_5.bits.CCM_CNV_21 = iL[1];
    outW.cnv_6.bits.CCM_CNV_22 = iL[2];
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_GGM(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_GGM_T& rGGM)
{
    (void)rCamInfo;
    (void)rGGM;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_YNRS(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_YNRS_T& rYNRS)
{
    (void)rCamInfo;
    (void)rYNRS;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_YNR(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_YNR_T& rYNR)
{
    (void)rCamInfo;
    (void)rYNR;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_LDNR(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_LDNR_T& rLDNR)
{
    (void)rCamInfo;
    (void)rLDNR;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_LCE(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_LCE_T& rLCE)
{
    (void)rCamInfo;
    (void)rLCE;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_DCE(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DCE_T& rDCE)
{
    (void)rCamInfo;
    (void)rDCE;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_EE(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_EE_T& rEE)
{
    (void)rCamInfo;
    (void)rEE;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_CNR(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CNR_CNR_T& rCNR)
{
    (void)rCamInfo;
    (void)rCNR;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_CCR(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CNR_CCR_T& rCCR)
{
    (void)rCamInfo;
    (void)rCCR;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_ABF(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CNR_ABF_T& rABF)
{
    (void)rCamInfo;
    (void)rABF;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_COLOR(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_COLOR_T& rCOLOR)
{
    (void)rCamInfo;
    (void)rCOLOR;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_MIX(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_MIX_T& rMIX)
{
    (void)rCamInfo;
    (void)rMIX;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_MFB(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_MFB_T& rMFB)
{
    (void)rCamInfo;
    (void)rMFB;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
IspTuningCustom::
refine_NR3D(MUINT8 SubModuleIndex, RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR3D_T& rNR3D)
{
    (void)rCamInfo;
    (void)rNR3D;

    switch (SubModuleIndex)
    {
        default:
            break;
    }
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
        rCamInfo.rMapping_Info.eIspProfile == Comm_EIspProfile_Capture_DCE || \
        rCamInfo.rMapping_Info.eIspProfile == Comm_EIspProfile_Capture_DSDN || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Capture_Capture_ZOOM1 || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Capture_Capture_ZOOM2 || \
        // flash capture
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Flash_Capture ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Flash_Capture_ZOOM1 || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Flash_Capture_ZOOM2 || \

        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Bokeh ||\
        // HDR
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_DCE ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_DSDN ||\

        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Capture || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Capture_DCE ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_mHDR_Capture_DSDN ||\

        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Capture ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Capture_DCE ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Capture_DSDN ||\

        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Capture ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Capture_DCE ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_zHDR_Capture_DSDN ||\

        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_zHDR_Capture_MFNR_After_Blend ||\

        // N3D
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Full ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Depth ||\
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_N3D_Capture_Depth_toW ||\
        // MFNR
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_MFNR_After_Blend || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_MFNR_After_ZOOM1 || \
        rCamInfo.rMapping_Info.eIspProfile == EIspProfile_MFNR_After_ZOOM2

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

    return MTRUE;
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
       //(rCamInfo.rMapping_Info.eIspProfile == EIspProfile_Auto_mHDR_Video)                  ||
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
        LSC_INPUT_INFO_T const& rLSCInfo
)   const
{
    MUINT32 EN_LSC_LOG_TSF_RUN = (1<<6);
    MUINT32 m_u4LogEn = property_get_int32("vendor.debug.lsc_mgr.log", 0);
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    MINT32 i4CCT = rLSCInfo.i4AWBCCT;

    EIndex_Shading_CCT_T eIdx_Shading_CCT_new = (EIndex_Shading_CCT_T)rLSCInfo.u4ShadingCCT;

//    -----------------|----|----|--------------|----|----|------------------
//                   THH2  TH2  THL2                   THH1  TH1  THL1

    MINT32 const THL1 = 3257;
    MINT32 const THH1 = 3484;
    MINT32 const TH1 = (THL1+THH1)/2; //(THL1 +THH1)/2
    MINT32 const THL2 = 4673;
    MINT32 const THH2 = 5155;
    MINT32 const TH2 = (THL2+THH2)/2;//(THL2 +THH2)/2

    switch  ((EIndex_Shading_CCT_T)rLSCInfo.u4ShadingCCT)
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

    if  ( (EIndex_Shading_CCT_T)rLSCInfo.u4ShadingCCT != eIdx_Shading_CCT_new )
    {
        MY_LOG_IF(fgLogEn,
            "[-evaluate_Shading_CCT_index] Shading CCT Idx(old,new)=(%d,%d), i4CCT = %d\n"
            , (EIndex_Shading_CCT_T)rLSCInfo.u4ShadingCCT, eIdx_Shading_CCT_new,i4CCT
        );
    }

    return  eIdx_Shading_CCT_new;
}


static MINT32 ratioMapping(MINT32 i4Iso, MINT32 i4Scenario)
{
#define LERP(x, lo_x, lo_y, hi_x, hi_y)\
    (((hi_x) - (x))*(lo_y) + ((x) - (lo_x))*(hi_y)) / ((hi_x) - (lo_x))

#define ASSIGN_VALUE(array, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9) \
{ \
    int tmp[10] = { v0, v1, v2, v3, v4, v5, v6, v7, v8, v9 };\
    for( int i = 0; i < 10; i++ ) \
        array[i] = tmp[i]; \
}

    static MINT32 iso[10] =
    {100, 200, 400, 800, 1200, 1600, 2000, 2400, 4800, 16000};

    static MINT32 rto[10] =
    //{24, 22, 20, 18, 16, 14, 12, 10, 8, 6}; //Tower modify for iso1600 Noise 2014-12-26
    //{30, 28, 26, 24, 22, 20, 18, 16, 14, 12};
    {32, 32, 32, 30, 28, 26, 26, 26, 26, 10};

// add your code here
    switch(i4Scenario)
    {
        //case EApp_SuperNight_Hand:
            //ASSIGN_VALUE(iso, 100, 200, 400, 800, 1200, 1600, 2000, 2400, 4800, 16000);
            //ASSIGN_VALUE(rto, 32,  32,  28,  24,   20,   16,   16,   16,   16,    10);
        //break;
        default:
            ASSIGN_VALUE(iso, 100, 200, 400, 800, 1200, 1600, 2000, 2400, 4800, 16000);
            ASSIGN_VALUE(rto, 32,  32,  32,  30,   28,   26,   26,   26,   26,    10);
        break;
    }
// end of add your code here

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
        for (i = 1; i < 9; i++) // i = 9 (max) when all other i4Iso >= iso[i]
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
        for (i = 1; i < mappingTableSize - 1; i++) // i = max index when all other i4EV >= adTBL_EV[i]
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
        LSC_INPUT_INFO_T const& rLSCInfo,
        MBOOL isSuperNightMode,
        MINT32 pParam
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
    MINT32 i = 0;
    MINT32 i4Rto = 8; //32;
    MINT32 i4Iso = rLSCInfo.u4RealISOValue;
    MINT32 i4Scenario = pParam;

    int idx = total_RA_num_frames_ % Avg_Frm_Cnt;
    int *p_global_Ra = ISO_Buffer_;
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
        if (rLSCInfo.isFlash == 1)
        {
            i4Rto = ratioMapping(i4Iso, i4Scenario);
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
                        OutSideAvgY += rLSCInfo.pu4AEBlock[i][j];
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

            lsc_new_version = rLSCInfo.bEnableRAFastConverge;
            MY_LOG_IF(fgLogEn,  "lsc_new_version=%d", lsc_new_version);

            if(lsc_new_version == 0)
            {
            i4Rto = ratioMapping(avgISO, i4Scenario);
            }
            else
            {
            int MAX_RA = 32;

            int i, j;
            int CWV1;
            int CWV2;
            int RA1 = 0;
            int RA2 = 0;
            int Dst_Ratio;
            int SlowLSC;
            int NonLinearLSC;
            float RA_Coeff = 0.0;
            int CWV1_Multiply_WeightSum;
            float LSC_GAIN_TABLE[AE_BLOCK_NO][AE_BLOCK_NO];
            int Pure_AE_Block[AE_BLOCK_NO][AE_BLOCK_NO];
            int Pure_New_AE_Block[AE_BLOCK_NO][AE_BLOCK_NO];
            int Pure_New_Weight_AE_Block[AE_BLOCK_NO][AE_BLOCK_NO];

            static int CrntRatio = 0;
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

            Dst_Ratio = ratioMapping(rLSCInfo.u4RealISOValue, i4Scenario);

            RA_Coeff = 0.0;
            RA1 = PreFrame2Ratio;
            CWV1 = rLSCInfo.u4MgrCWValue;

            if( ( CWV1>=230 || CWV1<=20) || rLSCInfo.TgCtrlRight==MTRUE  )
            {
                NonLinearLSC = 1;
            }
            else
            {
                NonLinearLSC = 0;
            }

            if( abs(rLSCInfo.i4deltaIndex) > 16*rLSCInfo.u4AEFinerEVIdxBase &&  NonLinearLSC==0 &&  !rLSCInfo.bAEStable)
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
                if(rLSCInfo.i4deltaIndex>0)
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
                if(rLSCInfo.i4deltaIndex > 0) //image is brighting, iso upperARA lower    --------|----------|---------|------
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
                else if(rLSCInfo.i4deltaIndex < 0) //image is darking, iso lowerARA uppder-------|-----------|--------|----
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
               rLSCInfo.u4RealISOValue, ratioMapping(rLSCInfo.u4RealISOValue, i4Scenario), i4Rto, rLSCInfo.u4MgrCWValue, rLSCInfo.u4AvgWValue, OutSideAvgY, rLSCInfo.TgCtrlRight*30, rLSCInfo.i4deltaIndex);
        }
    }

    MY_LOG_IF(isSuperNightMode,"[%s] before ratioMappingByEV ratio (%d)", __FUNCTION__, i4Rto);

    if(isSuperNightMode)
    {
        float ae_EV_Value = ((float)(rLSCInfo.i4AEComp)/(10.0));
        MY_LOG("[%s] ev in AE info is %.2f", __FUNCTION__, ae_EV_Value);
        i4Rto = ratioMappingByEV(i4Rto, ae_EV_Value, getSensorID());
    }

    MY_LOG_IF(isSuperNightMode,"[%s] after ratioMappingByEV ratio (%d)", __FUNCTION__, i4Rto);

    return i4Rto;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


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
map_Target_Size_to_MappingInfo(MUINT32 width, MUINT32 height, const char *pUserString){
   return ESize_SIZE_00;
}

MVOID
IspTuningCustom::
set_nvram_value(const NVRAM_CAMERA_ISP_PARAM_STRUCT* pIspParam) {
    memcpy(&m_Zoom_Env, &(pIspParam->ISPRegs.Zoom_Env), sizeof(ISP_NVRAM_ZOOM_INTERVAL_T));
}

MINT32
IspTuningCustom::
map_zoom_value_to_index(MINT32 i4ZoomRatio_x100){
    MUINT16 length = m_Zoom_Env.u2Length;

    if (length != 0){
        for(int i=0; i < length; i++){
            if(i4ZoomRatio_x100 <= m_Zoom_Env.IDX_Partition[i])
                return static_cast<EZoom_T>(i);
        }
        return static_cast<EZoom_T>(length-1);  //max
    } else
        return static_cast<EZoom_T>(0);

}

