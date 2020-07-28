/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef _CAMERA_CUSTOM_CAPTURE_NR_H_
#define _CAMERA_CUSTOM_CAPTURE_NR_H_

#include "camera_custom_types.h"

#define DISABLE_CAPTURE_NR (999999)

enum ESWNRType
{
    eSWNRType_SW,
    eSWNRType_SW2,
    eSWNRType_NUM,
};

enum SWNR_PerfLevel
{
    eSWNRPerf_Performance_First,
    eSWNRPerf_Custom0,
    eSWNRPerf_Power_First,
    eSWNRPerf_Default = eSWNRPerf_Performance_First,
};

typedef struct
{
    //int hwth;
    int swth;
}Capture_NR_Th_t;

typedef struct swnr_param
{
    MINT32 ANR_Y_LUMA_SCALE_RANGE;
    MINT32 ANR_C_CHROMA_SCALE;
	MINT32 ANR_Y_SCALE_CPY0;
	MINT32 ANR_Y_SCALE_CPY1;
	MINT32 ANR_Y_SCALE_CPY2;
	MINT32 ANR_Y_SCALE_CPY3;
	MINT32 ANR_Y_SCALE_CPY4;
	MINT32 ANR_Y_CPX1;
	MINT32 ANR_Y_CPX2;
	MINT32 ANR_Y_CPX3;
	MINT32 ANR_CEN_GAIN_LO_TH;
	MINT32 ANR_CEN_GAIN_HI_TH;
	MINT32 ANR_PTY_GAIN_TH;
	MINT32 ANR_KSIZE_LO_TH;
	MINT32 ANR_KSIZE_HI_TH;
	MINT32 ANR_KSIZE_LO_TH_C;
	MINT32 ANR_KSIZE_HI_TH_C;
	MINT32 ITUNE_ANR_PTY_STD;
	MINT32 ITUNE_ANR_PTU_STD;
	MINT32 ITUNE_ANR_PTV_STD;
	MINT32 ANR_ACT_TH_Y;
	MINT32 ANR_ACT_BLD_BASE_Y;
	MINT32 ANR_ACT_BLD_TH_Y;
	MINT32 ANR_ACT_SLANT_Y;
	MINT32 ANR_ACT_TH_C;
	MINT32 ANR_ACT_BLD_BASE_C;
	MINT32 ANR_ACT_BLD_TH_C;
	MINT32 ANR_ACT_SLANT_C;
	MINT32 RADIUS_H;
	MINT32 RADIUS_V;
	MINT32 RADIUS_H_C;
	MINT32 RADIUS_V_C;
	MINT32 ANR_PTC_HGAIN;
	MINT32 ANR_PTY_HGAIN;
	MINT32 ANR_LPF_HALFKERNEL;
	MINT32 ANR_LPF_HALFKERNEL_C;
	MINT32 ANR_ACT_MODE;
	MINT32 ANR_LCE_SCALE_GAIN;
	MINT32 ANR_LCE_C_GAIN;
	MINT32 ANR_LCE_GAIN0;
	MINT32 ANR_LCE_GAIN1;
	MINT32 ANR_LCE_GAIN2;
	MINT32 ANR_LCE_GAIN3;
	MINT32 ANR_MEDIAN_LOCATION;
	MINT32 ANR_CEN_X;
	MINT32 ANR_CEN_Y;
	MINT32 ANR_R1;
	MINT32 ANR_R2;
	MINT32 ANR_R3;
    MINT32 LUMA_ON_OFF;	
}swnr_param_t;

typedef struct swnr2_param
{
    MINT32 NR_K;
    MINT32 NR_S;
	MINT32 NR_SD;
    MINT32 NR_BLD_W;
    MINT32 NR_BLD_TH;
    MINT32 NR_SMTH;	
}swnr2_param_t;

typedef struct hfg_param
{
    MINT32 HFG_ENABLE;
    MINT32 HFG_GSD;
	MINT32 HFG_SD0;
    MINT32 HFG_SD1;
    MINT32 HFG_SD2;
    MINT32 HFG_TX_S;
    MINT32 HFG_LCE_LINK_EN;
    MINT32 HFG_LUMA_CPX1;
    MINT32 HFG_LUMA_CPX2;
    MINT32 HFG_LUMA_CPX3;
    MINT32 HFG_LUMA_CPY0;
    MINT32 HFG_LUMA_CPY1;
    MINT32 HFG_LUMA_CPY2;
    MINT32 HFG_LUMA_CPY3;
    MINT32 HFG_LUMA_SP0;
    MINT32 HFG_LUMA_SP1;
    MINT32 HFG_LUMA_SP2;
    MINT32 HFG_LUMA_SP3;
}hfg_param_t;

typedef struct ccr_param
{
    MINT32 CCR_ENABLE;
    MINT32 CCR_CEN_U;
	MINT32 CCR_CEN_V;
    MINT32 CCR_Y_CPX1;
    MINT32 CCR_Y_CPX2;
    MINT32 CCR_Y_CPY1;
    MINT32 CCR_Y_SP1;
    MINT32 CCR_UV_X1;
    MINT32 CCR_UV_X2;
    MINT32 CCR_UV_X3;
    MINT32 CCR_UV_GAIN1;
    MINT32 CCR_UV_GAIN2;
    MINT32 CCR_UV_GAIN_SP1;
    MINT32 CCR_UV_GAIN_SP2;
    MINT32 CCR_Y_CPX3;
    MINT32 CCR_Y_CPY0;
    MINT32 CCR_Y_CPY2;
    MINT32 CCR_Y_SP0;
    MINT32 CCR_Y_SP2;
    MINT32 CCR_UV_GAIN_MODE;
    MINT32 CCR_MODE;
    MINT32 CCR_OR_MODE;
    MINT32 CCR_HUE_X1;
    MINT32 CCR_HUE_X2;
    MINT32 CCR_HUE_X3;
    MINT32 CCR_HUE_X4;
    MINT32 CCR_HUE_SP1;
    MINT32 CCR_HUE_SP2;
    MINT32 CCR_HUE_GAIN1;
    MINT32 CCR_HUE_GAIN2;	
}ccr_param_t;

enum EIdxSwNR
{
    eIDX_SWNR_SINGLE_ISO_100 = 0,
    eIDX_SWNR_SINGLE_ISO_200,
    eIDX_SWNR_SINGLE_ISO_400,
    eIDX_SWNR_SINGLE_ISO_800,
    eIDX_SWNR_SINGLE_ISO_1200,
    eIDX_SWNR_SINGLE_ISO_1600,
    eIDX_SWNR_SINGLE_ISO_2000,
    eIDX_SWNR_SINGLE_ISO_2400,
    eIDX_SWNR_SINGLE_ISO_2800,
    eIDX_SWNR_SINGLE_ISO_3200,
    eNUM_OF_SWNR_IDX,
};

bool get_capture_nr_th(
        MUINT32 const sensorDev,
        MUINT32 const shotmode,
        Capture_NR_Th_t* pTh
        );

bool get_tuning_param_NR(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        swnr_param_t& param
        );

bool get_tuning_param_NR2(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        swnr2_param_t& param
        );

bool get_tuning_param_HFG(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        hfg_param_t& param
        );

bool get_tuning_param_CCR(
        MUINT32 const sensorDev,
        MUINT32 const u4Idx,
        ccr_param_t& param
        );

bool smooth_param(
        MUINT32 const u4RealISO,
        MUINT32 const u4UpperISO,
        MUINT32 const u4LowerISO,
        swnr_param_t const& rUpperParam,
        swnr_param_t const& rLowerParam,
        swnr_param_t & rSmoothParam
        );

bool smooth_param_NR2(
        MUINT32 const u4RealISO,
        MUINT32 const u4UpperISO,
        MUINT32 const u4LowerISO,
        swnr2_param_t const& rUpperParam,
        swnr2_param_t const& rLowerParam,
        swnr2_param_t & rSmoothParam
        );


bool refineByScale(
        MUINT32 const sensorDev,
        MUINT32 const scale_x100,
        swnr_param_t & rSmoothParam
        );

bool refineByScaleNR2(
        MUINT32 const sensorDev,
        MUINT32 const scale_x100,
        swnr2_param_t & rSmoothParam
        );

MINT32 get_performance_level(
        MUINT32 const sensorDev,
        MUINT32 const shotmode
        );

MUINT32 map_ISO_value_to_index(MUINT32 u4Iso);
MUINT32 map_index_to_ISO_value(MUINT32 u4Idx);
MBOOL is_to_invoke_swnr_interpolation(MUINT32 const u4Iso);
MINT32 get_swnr_type(MUINT32 sensorDev);

#endif /* _CAMERA_CUSTOM_CAPTURE_NR_H_ */
