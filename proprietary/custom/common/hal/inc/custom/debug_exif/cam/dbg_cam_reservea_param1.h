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

#pragma once

/******************************************************************************
 *
 ******************************************************************************/
#include "../dbg_exif_def.h"

namespace dbg_cam_reservea_param_1 {
/******************************************************************************
 *
 ******************************************************************************/

// TEST_A debug info
enum { RESERVEA_DEBUG_TAG_VERSION = 1 };
enum { DBG_NR_SIZE = 98 };  // max(MNR, SWNR, fSWNR)
enum { RESERVEA_DEBUG_TAG_SIZE = (1/*version*/+DBG_NR_SIZE/*NRs*/) };

//ReserveA Parameter Structure
typedef enum
{
    RESERVEA_TAG_VERSION = 0,
    /* add tags here */
    // MNR related: ANR + ANR2 + CCR
    M_IspProfile,
    M_IDX_ANR,
    M_ANR_UPPER_ISO,
    M_ANR_LOWER_ISO,
    M_ANR_UPPER_IDX,
    M_ANR_LOWER_IDX,
    M_IDX_ANR2,
    M_ANR2_UPPER_ISO,
    M_ANR2_LOWER_ISO,
    M_ANR2_UPPER_IDX,
    M_ANR2_LOWER_IDX,
    M_IDX_CCR,
    M_CCR_UPPER_ISO,
    M_CCR_LOWER_ISO,
    M_CCR_UPPER_IDX,
    M_CCR_LOWER_IDX,
    M_DIP_X_CTL_YUV_EN,
    //  ANR (35)
    M_DIP_X_ANR_CON1,
    M_DIP_X_ANR_CON2,
    M_DIP_X_ANR_YAD1,
    M_DIP_X_ANR_YAD2,
    M_DIP_X_ANR_Y4LUT1,
    M_DIP_X_ANR_Y4LUT2,
    M_DIP_X_ANR_Y4LUT3,
    M_DIP_X_ANR_C4LUT1,
    M_DIP_X_ANR_C4LUT2,
    M_DIP_X_ANR_C4LUT3,
    M_DIP_X_ANR_A4LUT2,
    M_DIP_X_ANR_A4LUT3,
    M_DIP_X_ANR_L4LUT1,
    M_DIP_X_ANR_L4LUT2,
    M_DIP_X_ANR_L4LUT3,
    M_DIP_X_ANR_PTY,
    M_DIP_X_ANR_CAD,
    M_DIP_X_ANR_PTC,
    M_DIP_X_ANR_LCE,
    M_DIP_X_ANR_MED1,
    M_DIP_X_ANR_MED2,
    M_DIP_X_ANR_MED3,
    M_DIP_X_ANR_MED4,
    M_DIP_X_ANR_HP1,
    M_DIP_X_ANR_HP2,
    M_DIP_X_ANR_HP3,
    M_DIP_X_ANR_ACT1,
    M_DIP_X_ANR_ACT2,
    M_DIP_X_ANR_ACT3,
    M_DIP_X_ANR_ACTYH,
    M_DIP_X_ANR_ACTC,
    M_DIP_X_ANR_ACTYL,
    M_DIP_X_ANR_YLAD,
    M_DIP_X_ANR_PTYL,
    M_DIP_X_ANR_LCOEF,
    //
    //  ANR2 (18)
    M_DIP_X_ANR2_CON1,
    M_DIP_X_ANR2_CON2,
    M_DIP_X_ANR2_YAD1,
    M_DIP_X_ANR2_Y4LUT1,
    M_DIP_X_ANR2_Y4LUT2,
    M_DIP_X_ANR2_Y4LUT3,
    M_DIP_X_ANR2_L4LUT1,
    M_DIP_X_ANR2_L4LUT2,
    M_DIP_X_ANR2_L4LUT3,
    M_DIP_X_ANR2_CAD,
    M_DIP_X_ANR2_PTC,
    M_DIP_X_ANR2_LCE,
    M_DIP_X_ANR2_MED1,
    M_DIP_X_ANR2_MED2,
    M_DIP_X_ANR2_MED3,
    M_DIP_X_ANR2_MED4,
    M_DIP_X_ANR2_ACTY,
    M_DIP_X_ANR2_ACTC,
    //
    //  CCR (9)
    M_DIP_X_CCR_CON,
    M_DIP_X_CCR_YLUT,
    M_DIP_X_CCR_UVLUT,
    M_DIP_X_CCR_YLUT2,
    M_DIP_X_CCR_SAT_CTRL,
    M_DIP_X_CCR_UVLUT_SP,
    M_DIP_X_CCR_HUE1,
    M_DIP_X_CCR_HUE2,
    M_DIP_X_CCR_HUE3,
    //
    // SWNR related
    //  1. SWNR: NR + HFG + CCR
    //  NR (50)
    ANR_Y_LUMA_SCALE_RANGE,
    ANR_C_CHROMA_SCALE,
    ANR_Y_SCALE_CPY0,
    ANR_Y_SCALE_CPY1,
    ANR_Y_SCALE_CPY2,
    ANR_Y_SCALE_CPY3,
    ANR_Y_SCALE_CPY4,
    ANR_Y_CPX1,
    ANR_Y_CPX2,
    ANR_Y_CPX3,
    ANR_CEN_GAIN_LO_TH,
    ANR_CEN_GAIN_HI_TH,
    ANR_PTY_GAIN_TH,
    ANR_KSIZE_LO_TH,
    ANR_KSIZE_HI_TH,
    ANR_KSIZE_LO_TH_C,
    ANR_KSIZE_HI_TH_C,
    ITUNE_ANR_PTY_STD,
    ITUNE_ANR_PTU_STD,
    ITUNE_ANR_PTV_STD,
    ANR_ACT_TH_Y,
    ANR_ACT_BLD_BASE_Y,
    ANR_ACT_BLD_TH_Y,
    ANR_ACT_SLANT_Y,
    ANR_ACT_TH_C,
    ANR_ACT_BLD_BASE_C,
    ANR_ACT_BLD_TH_C,
    ANR_ACT_SLANT_C,
    RADIUS_H,
    RADIUS_V,
    RADIUS_H_C,
    RADIUS_V_C,
    ANR_PTC_HGAIN,
    ANR_PTY_HGAIN,
    ANR_LPF_HALFKERNEL,
    ANR_LPF_HALFKERNEL_C,
    ANR_ACT_MODE,
    ANR_LCE_SCALE_GAIN,
    ANR_LCE_C_GAIN,
    ANR_LCE_GAIN0,
    ANR_LCE_GAIN1,
    ANR_LCE_GAIN2,
    ANR_LCE_GAIN3,
    ANR_MEDIAN_LOCATION,
    ANR_CEN_X,
    ANR_CEN_Y,
    ANR_R1,
    ANR_R2,
    ANR_R3,
    LUMA_ON_OFF,
    // 2. fast swnr: NR2 + HFG + CCR
    // NR2 (6)
    NR_K,
    NR_S,
    NR_SD,
    NR_BLD_W,
    NR_BLD_TH,
    NR_SMTH,
    // HFG (18)
    HFG_ENABLE,
    HFG_GSD,
    HFG_SD0,
    HFG_SD1,
    HFG_SD2,
    HFG_TX_S,
    HFG_LCE_LINK_EN,
    HFG_LUMA_CPX1,
    HFG_LUMA_CPX2,
    HFG_LUMA_CPX3,
    HFG_LUMA_CPY0,
    HFG_LUMA_CPY1,
    HFG_LUMA_CPY2,
    HFG_LUMA_CPY3,
    HFG_LUMA_SP0,
    HFG_LUMA_SP1,
    HFG_LUMA_SP2,
    HFG_LUMA_SP3,
    // CCR (30)
    CCR_ENABLE,
    CCR_CEN_U,
    CCR_CEN_V,
    CCR_Y_CPX1,
    CCR_Y_CPX2,
    CCR_Y_CPY1,
    CCR_Y_SP1,
    CCR_UV_X1,
    CCR_UV_X2,
    CCR_UV_X3,
    CCR_UV_GAIN1,
    CCR_UV_GAIN2,
    CCR_UV_GAIN_SP1,
    CCR_UV_GAIN_SP2,
    CCR_Y_CPX3,
    CCR_Y_CPY0,
    CCR_Y_CPY2,
    CCR_Y_SP0,
    CCR_Y_SP2,
    CCR_UV_GAIN_MODE,
    CCR_MODE,
    CCR_OR_MODE,
    CCR_HUE_X1,
    CCR_HUE_X2,
    CCR_HUE_X3,
    CCR_HUE_X4,
    CCR_HUE_SP1,
    CCR_HUE_SP2,
    CCR_HUE_GAIN1,
    CCR_HUE_GAIN2,
    //
    RESERVEA_TAG_END
}DEBUG_RESERVEA_TAG_T;

struct DEBUG_RESERVEA_INFO_T {
    debug_exif_field Tag[RESERVEA_DEBUG_TAG_SIZE];
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace

