/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _FSC_CUSTOM_BASE_H_
#define _FSC_CUSTOM_BASE_H_

#include <cutils/properties.h>
#include <math.h>
#include <mtkcam/def/BuiltinTypes.h>

#define FSC_FSC_ENABLE_PROPERTY                 "vendor.debug.fsc.fsc.enable"
#define FSC_DEBUG_LEVEL_PROPERTY                "vendor.debug.fsc.debug.level"
#define FSC_MARCO_INF_RATIO_OFFSET_PROPERTY     "vendor.debug.fsc.marco_inf_ratio.offset"
#define FSC_MAX_CROP_RATIO_PROPERTY             "vendor.debug.fsc.max_crop.ratio"
#define FSC_AF_DAMPTIME_OFFSET_PROPERTY         "vendor.debug.fsc.af_damp_time.offset"

#define isFSCUsageMaskEnable(x, mask)        ((x) & (mask))

class FSCCustomBase
{
public:
    enum USAGE_MASK
    {
        USAGE_MASK_NONE         = 0,
        USAGE_MASK_DUAL_ZOOM    = 1 << 0,
        USAGE_MASK_MULTIUSER    = 1 << 1,
        USAGE_MASK_HIGHSPEED    = 1 << 2,
    };

protected:
    // DO NOT create instance
    FSCCustomBase()
    {
    }

public:
    /* Get FSC Mode
     */
    static MBOOL isSupportFSC()
    {
        // return MTRUE if platform support RSC
        return false;
    }

    static MBOOL isEnabledFSC(MUINT32 mask)
    {
        // return MTRUE if RSC can enable RSC in usage of mask
        (void)mask;
        return false;
    }

    static MINT32 getDebugLevel()
    {
        return 0;
    }

    static MINT32 getMarcoToInfRatioOffset()
    {
        // 10000 = ratio 1.0
        return 0;
    }

    static MINT32 getMaxCropRatio()
    {
        // return 1000 = 10% = 0.1 * 10000
        return 1000;
    }

    static MINT32 getAfDampTimeOffset()
    {
        // 1000 = 1ms
        return 0;
    }

};
#endif /* _FSC_CUSTOM_BASE_H_ */

