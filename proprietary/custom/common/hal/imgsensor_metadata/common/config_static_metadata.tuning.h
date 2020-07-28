/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


STATIC_METADATA2_BEGIN(COMMON, TUNING, COMMON)
//------------------------------------------------------------------------------
//  android.control
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_COLOR_CORRECTION_AVAILABLE_ABERRATION_MODES)
        CONFIG_ENTRY_VALUE(MTK_COLOR_CORRECTION_ABERRATION_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AVAILABLE_MODES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_AUTO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_USE_SCENE_MODE, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_MODE_OFF_KEEP_STATE, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AVAILABLE_EFFECTS)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_OFF, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_MONO, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_NEGATIVE, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_SOLARIZE, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_SEPIA, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_POSTERIZE, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_WHITEBOARD, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_BLACKBOARD, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_CONTROL_EFFECT_MODE_AQUA, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_MAX_REGIONS)
        CONFIG_ENTRY_VALUE(1, MINT32)
        CONFIG_ENTRY_VALUE(1, MINT32)
        CONFIG_ENTRY_VALUE(0, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_AVAILABLE_SCENE_MODES)
    CONFIG_ENTRY_VALUE(MTK_CONTROL_SCENE_MODE_DISABLED, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_CONTROL_SCENE_MODE_OVERRIDES)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AE_MODE_ON, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AWB_MODE_AUTO, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_CONTROL_AF_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.shading
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SHADING_AVAILABLE_MODES)
        CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_FAST, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_SHADING_MODE_HIGH_QUALITY, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.statistics.info
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT)
        CONFIG_ENTRY_VALUE(64, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_MAX_HISTOGRAM_COUNT)
        CONFIG_ENTRY_VALUE(1000, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_MAX_SHARPNESS_MAP_VALUE)
        CONFIG_ENTRY_VALUE(1000, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_SHARPNESS_MAP_SIZE)
        CONFIG_ENTRY_VALUE(MSize(64, 64), MSize)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_AVAILABLE_HOT_PIXEL_MAP_MODES)
        CONFIG_ENTRY_VALUE(MTK_STATISTICS_HOT_PIXEL_MAP_MODE_OFF, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES)
        CONFIG_ENTRY_VALUE(MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.tonemap
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_TONEMAP_MAX_CURVE_POINTS)
        CONFIG_ENTRY_VALUE(101, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_TONEMAP_AVAILABLE_TONE_MAP_MODES)
        CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_CONTRAST_CURVE, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_FAST, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_HIGH_QUALITY, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_GAMMA_VALUE, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_TONEMAP_MODE_PRESET_CURVE, MUINT8)
    CONFIG_METADATA_END()
//------------------------------------------------------------------------------
//  android.edge
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_EDGE_AVAILABLE_EDGE_MODES)
        CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_FAST, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_HIGH_QUALITY, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_EDGE_MODE_ZERO_SHUTTER_LAG, MUINT8)   // auto gen if reproc
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.hotPixel
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_HOT_PIXEL_AVAILABLE_HOT_PIXEL_MODES)
        CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_FAST, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_HOT_PIXEL_MODE_HIGH_QUALITY, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.noiseReduction
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES)
        CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_OFF, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_FAST, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_MINIMAL, MUINT8)
        // CONFIG_ENTRY_VALUE(MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG, MUINT8) // auto gen if reproc
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
STATIC_METADATA_END()

