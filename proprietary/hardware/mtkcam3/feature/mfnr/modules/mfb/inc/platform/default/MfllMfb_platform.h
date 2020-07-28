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
#ifndef __MFLLMFB_PLATFORM_H__
#define __MFLLMFB_PLATFORM_H__

#include "MfllIspProfiles.h"

#include <mtkcam3/feature/mfnr/IMfllCore.h>

namespace mfll
{

static bool workaround_MFB_stage(void* regs __attribute__((unused)))
{
    return true;
}


static void debug_pass2_registers(
        void*   regs    __attribute__((unused)),
        int     stage   __attribute__((unused))
        )
{
}

static void dump_exif_info(
        IMfllCore*  c,
        void*       regs,
        int         stage)
{
}

static EIspProfile_T get_isp_profile(
        const eMfllIspProfile&, /* p */
        MfllMode                /* mode = MfllMode_ZsdMfll*/)
{
    return static_cast<EIspProfile_T>(MFLL_ISP_PROFILE_ERROR);
}


// cfg is in/out
inline bool refine_register(MfbPlatformConfig& cfg)
{
    return true;
}

/**
 *  Describe the pass 2 port ID
 */
#define PASS2_CROPPING_ID_CRZ       1
#define PASS2_CROPPING_ID_WDMAO     2
#define PASS2_CROPPING_ID_WROTO     3
/******************************************************************************
 *  RAW2YUV
 *****************************************************************************/
#define RAW2YUV_PORT_IN             PORT_IMGI
#define RAW2YUV_PORT_LCE_IN         PORT_LCEI
#define RAW2YUV_PORT_PBC_IN         PORT_DMGI

// half size MC
#define RAW2YUV_PORT_OUT            PORT_WDMAO

// full size MC
#define RAW2YUV_PORT_OUT_NO_CRZ     PORT_IMG3O  // using IMG3O for the best quality and bit true
#define RAW2YUV_PORT_OUT2           PORT_WROTO
#define RAW2YUV_PORT_OUT3           PORT_WDMAO

/* Cropping group ID is related port ID, notice this ... */
#define RAW2YUV_GID_OUT             PASS2_CROPPING_ID_WDMAO
#define RAW2YUV_GID_OUT2            PASS2_CROPPING_ID_WROTO
#define RAW2YUV_GID_OUT3            PASS2_CROPPING_ID_WDMAO

#define MIX_MAIN_GID_OUT            PASS2_CROPPING_ID_WDMAO
#define MIX_THUMBNAIL_GID_OUT       PASS2_CROPPING_ID_WROTO

/******************************************************************************
 *  MFB
 *****************************************************************************/
/* port */
#define MFB_PORT_IN_BASE_FRAME      PORT_IMGI
#define MFB_PORT_IN_REF_FRAME       PORT_IMGBI
#define MFB_PORT_IN_WEIGHTING       PORT_IMGCI
#define MFB_PORT_IN_CONF_MAP        PORT_LCEI
#define MFB_PORT_OUT_FRAME          PORT_IMG3O // using IMG3O for the best quality and bit true
#define MFB_PORT_OUT_WEIGHTING      PORT_MFBO
#define MFB_SUPPORT_CONF_MAP        0

/* group ID in MFB stage, if not defined which means not support crop */
#define MFB_GID_OUT_FRAME           1 // IMG2O group id in MFB stage

/******************************************************************************
 *  MIX
 *****************************************************************************/
/* port */
#define MIX_PORT_IN_BASE_FRAME      PORT_IMGI
#define MIX_PORT_IN_GOLDEN_FRAME    PORT_VIPI
#define MIX_PORT_IN_WEIGHTING       PORT_IMGBI
#define MIX_PORT_OUT_FRAME          PORT_IMG3O // using IMG3O for the best quality and bit true
#define MIX_PORT_OUT_MAIN           PORT_WDMAO // using IMG3O for the best quality and bit true
#define MIX_PORT_OUT_THUMBNAIL      PORT_WROTO

};/* namespace mfll */
#endif//__MFLLMFB_PLATFORM_H__
