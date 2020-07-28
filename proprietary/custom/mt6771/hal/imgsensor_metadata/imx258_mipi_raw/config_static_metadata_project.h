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


STATIC_METADATA2_BEGIN(PLATFORM, PROJECT, SENSOR_DRVNAME_IMX258_MIPI_RAW)
//------------------------------------------------------------------------------
//  android.request
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_REQUEST_AVAILABLE_CAPABILITIES)//new
        CONFIG_ENTRY_VALUE(MTK_REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_REQUEST_AVAILABLE_CAPABILITIES_MANUAL_SENSOR, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_REQUEST_AVAILABLE_CAPABILITIES_MANUAL_POST_PROCESSING, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_REQUEST_AVAILABLE_CAPABILITIES_READ_SENSOR_SETTINGS, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_REQUEST_AVAILABLE_CAPABILITIES_RAW, MUINT8)
        CONFIG_ENTRY_VALUE(MTK_REQUEST_AVAILABLE_CAPABILITIES_BURST_CAPTURE, MUINT8)
    CONFIG_METADATA_END()

//------------------------------------------------------------------------------
//  android.scaler
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_WITH_DURATIONS)

        CONFIG_ENTRY_VALUE(HAL_PIXEL_FORMAT_BLOB, MINT64) //ADD
        CONFIG_ENTRY_VALUE(4208, MINT64)
        CONFIG_ENTRY_VALUE(3120, MINT64)
        CONFIG_ENTRY_VALUE(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT, MINT64)
        CONFIG_ENTRY_VALUE(66666666, MINT64)    //Frame_Durations
        CONFIG_ENTRY_VALUE(33333333, MINT64)    //Stall_Durations

        CONFIG_ENTRY_VALUE(HAL_PIXEL_FORMAT_RAW16, MINT64) //ADD
        CONFIG_ENTRY_VALUE(4208, MINT64)
        CONFIG_ENTRY_VALUE(3120, MINT64)
        CONFIG_ENTRY_VALUE(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT, MINT64)
        CONFIG_ENTRY_VALUE(33333333, MINT64)    //Frame_Duration
        CONFIG_ENTRY_VALUE(33333333, MINT64)    //Stall_Durations

        CONFIG_ENTRY_VALUE(HAL_PIXEL_FORMAT_YCbCr_420_888, MINT64) //ADD
        CONFIG_ENTRY_VALUE(4208, MINT64)
        CONFIG_ENTRY_VALUE(3120, MINT64)
        CONFIG_ENTRY_VALUE(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT, MINT64)
        CONFIG_ENTRY_VALUE(100000000, MINT64)    //Frame_Duration
        CONFIG_ENTRY_VALUE(0, MINT64)           //Stall_Durations

        CONFIG_ENTRY_VALUE(HAL_PIXEL_FORMAT_YCbCr_420_888, MINT64)
        CONFIG_ENTRY_VALUE(1920, MINT64)
        CONFIG_ENTRY_VALUE(1088, MINT64)
        CONFIG_ENTRY_VALUE(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT, MINT64)
        CONFIG_ENTRY_VALUE(100000000, MINT64)    //Frame_Duration
        CONFIG_ENTRY_VALUE(0, MINT64)           //Stall_Durations


    CONFIG_METADATA_END()
    //==========================================================================

//------------------------------------------------------------------------------
//  android.reprocess
//------------------------------------------------------------------------------
    //==========================================================================
    // CONFIG_METADATA_BEGIN(MTK_REPROCESS_MAX_CAPTURE_STALL)
        // CONFIG_ENTRY_VALUE(1 , MINT32)
    // CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.jpeg
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
STATIC_METADATA_END()


STATIC_METADATA2_BEGIN(PLATFORM, PRO_OVERWRITE, SENSOR_DRVNAME_IMX258_MIPI_RAW)
// preserver for future use
STATIC_METADATA_END()
