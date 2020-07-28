/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __MFLLISPPROFILES_H__
#define __MFLLISPPROFILES_H__
//

namespace mfll
{
///////////////////////////////////////////////////////////////////////////////
// MFNR stage ID
///////////////////////////////////////////////////////////////////////////////
#define STAGE_BASE_YUV    0
#define STAGE_MFB         1
#define STAGE_GOLDEN_YUV  2
#define STAGE_MIX         3
#define STAGE_SIZE        4


///////////////////////////////////////////////////////////////////////////////
// Platform dependent configurations
///////////////////////////////////////////////////////////////////////////////
enum ePLATFORMCFG
{
    ePLATFORMCFG_REFINE_OK,         // represents if refine_register returns ok
    ePLATFORMCFG_STAGE,             // stage, see STAGE_BASE_YUV...etc
    ePLATFORMCFG_INDEX,             // frame index, value
    ePLATFORMCFG_DIP_X_REG_T,       // pointer of dip_x_reg_t

    ePLATFORMCFG_FULL_SIZE_MC,      // 0 for half size MC
    ePLATFORMCFG_SRC_WIDTH,         // width of source image
    ePLATFORMCFG_SRC_HEIGHT,        // height of source image

    ePLATFORMCFG_ENABLE_MIX3,       // to read out if MIX3 is enabled or not
};
typedef std::unordered_map<ePLATFORMCFG, intptr_t> MfbPlatformConfig;


///////////////////////////////////////////////////////////////////////////////
// The ISP profile that MFNR may use.
// To avoid platform dependent part, we re-map ISP profiles here.
///////////////////////////////////////////////////////////////////////////////
typedef enum {
    // stage: bfbld
    eMfllIspProfile_BeforeBlend = 0, // normal case
    eMfllIspProfile_BeforeBlend_Swnr, // for SWNR enabled
    eMfllIspProfile_BeforeBlend_Zsd, // zsd case (time sharing)
    eMfllIspProfile_BeforeBlend_Zsd_Swnr, // for zsd+SWNR enabled

    // stage: single
    eMfllIspProfile_Single,
    eMfllIspProfile_Single_Swnr,
    eMfllIspProfile_Single_Zsd,
    eMfllIspProfile_Single_Zsd_Swnr,

    // stage: mfb
    eMfllIspProfile_Mfb,
    eMfllIspProfile_Mfb_Swnr,
    eMfllIspProfile_Mfb_Zsd,
    eMfllIspProfile_Mfb_Zsd_Swnr,

    // stage: mix (after blend)
    eMfllIspProfile_AfterBlend,
    eMfllIspProfile_AfterBlend_Swnr,
    eMfllIspProfile_AfterBlend_Zsd,
    eMfllIspProfile_AfterBlend_Zsd_Swnr,

    // represents size
    eMfllIspProfile_Size,
} eMfllIspProfile;

#define MFLL_ISP_PROFILE_ERROR  0xFFFFFFFF

};/* namespace mfll */
#endif//__MFLLISPPROFILES_H__
