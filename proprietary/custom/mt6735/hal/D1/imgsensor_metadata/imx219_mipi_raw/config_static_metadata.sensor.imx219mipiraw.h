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
 * MediaTek Inc. (C) 2010. All rights reserved.
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


STATIC_METADATA2_BEGIN(DEVICE, SENSOR, SENSOR_DRVNAME_IMX219_MIPI_RAW)
//------------------------------------------------------------------------------
//  android.sensor
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_BLACK_LEVEL_PATTERN)
        CONFIG_ENTRY_VALUE(64, MINT32)
        CONFIG_ENTRY_VALUE(64, MINT32)
        CONFIG_ENTRY_VALUE(64, MINT32)
        CONFIG_ENTRY_VALUE(64, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
//------------------------------------------------------------------------------
//  android.sensor.info
//------------------------------------------------------------------------------
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION)
        CONFIG_ENTRY_VALUE(MRect(MPoint(4, 4), MSize(3264, 2448)), MRect)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_SENSITIVITY_RANGE)
        CONFIG_ENTRY_VALUE(100, MINT32)
        CONFIG_ENTRY_VALUE(6400, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT)
        CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT_BGGR, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE)// 1 us - 30 sec
        CONFIG_ENTRY_VALUE(100000L, MINT64)
        CONFIG_ENTRY_VALUE(400000000L, MINT64)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_MAX_FRAME_DURATION)// 30 sec
        CONFIG_ENTRY_VALUE(400000000L, MINT64)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PHYSICAL_SIZE) // mm
        CONFIG_ENTRY_VALUE(3.68f, MFLOAT)
        CONFIG_ENTRY_VALUE(2.76f, MFLOAT)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE)
        CONFIG_ENTRY_VALUE(MSize(3448, 2544), MSize)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_WHITE_LEVEL)
        CONFIG_ENTRY_VALUE(4000, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_TIMESTAMP_SOURCE)
        CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_TIMESTAMP_SOURCE_UNKNOWN, MUINT8)
        //CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_TIMESTAMP_SOURCE_REALTIME, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_MAX_ANALOG_SENSITIVITY)
        CONFIG_ENTRY_VALUE(240, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_BASE_GAIN_FACTOR)
        CONFIG_ENTRY_VALUE(MRational(42, 256), MRational)
      CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_ORIENTATION)
        CONFIG_ENTRY_VALUE(    90, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_PROFILE_HUE_SAT_MAP_DIMENSIONS)
        CONFIG_ENTRY_VALUE(    1, MINT32)
        CONFIG_ENTRY_VALUE(    2, MINT32)
        CONFIG_ENTRY_VALUE(    1, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_AVAILABLE_TEST_PATTERN_MODES)
        CONFIG_ENTRY_VALUE(    0, MINT32)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_INFO_PACKAGE)
        //----------------------------------------------------------------------
        CONFIG_ENTRY_METADATA(
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_SCENARIO_ID)
                CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_PREVIEW, MINT32)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_FRAME_RATE)
                CONFIG_ENTRY_VALUE(30, MINT32)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY)
                CONFIG_ENTRY_VALUE(MRect(MPoint(2, 2), MSize(3264, 2448)),MRect)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE)
                CONFIG_ENTRY_VALUE(MSize(1632,  1224),MSize)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_ROLLING_SHUTTER_SKEW)
                CONFIG_ENTRY_VALUE(33000000,MINT64) // nanosecond
            CONFIG_METADATA2_END()
            //..................................................................
        )
        //----------------------------------------------------------------------
        CONFIG_ENTRY_METADATA(
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_SCENARIO_ID)
                CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_CAPTURE, MINT32)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_FRAME_RATE)
                CONFIG_ENTRY_VALUE(30, MINT32)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY)
                CONFIG_ENTRY_VALUE(MRect(MPoint(4, 4), MSize(3264, 2448)),MRect)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE)
                CONFIG_ENTRY_VALUE(MSize(3264, 2448),MSize)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_ROLLING_SHUTTER_SKEW)
                CONFIG_ENTRY_VALUE(33000000,MINT64) // nanosecond
            CONFIG_METADATA2_END()
            //..................................................................
        )
        //----------------------------------------------------------------------
        CONFIG_ENTRY_METADATA(
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_SCENARIO_ID)
                CONFIG_ENTRY_VALUE(MTK_SENSOR_INFO_SCENARIO_ID_NORMAL_VIDEO, MINT32)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_FRAME_RATE)
                CONFIG_ENTRY_VALUE(30, MINT32)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY)
                CONFIG_ENTRY_VALUE(MRect(MPoint(4, 4), MSize(3264, 2448)),MRect)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE)
                CONFIG_ENTRY_VALUE(MSize(3264, 2448),MSize)
            CONFIG_METADATA2_END()
            //..................................................................
            CONFIG_METADATA2_BEGIN(MTK_SENSOR_ROLLING_SHUTTER_SKEW)
                CONFIG_ENTRY_VALUE(33000000,MINT64) // nanosecond
            CONFIG_METADATA2_END()
            //..................................................................
        )
        //----------------------------------------------------------------------
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_REFERENCE_ILLUMINANT1)
        CONFIG_ENTRY_VALUE(MTK_SENSOR_REFERENCE_ILLUMINANT1_DAYLIGHT, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_REFERENCE_ILLUMINANT2)
        CONFIG_ENTRY_VALUE(MTK_SENSOR_REFERENCE_ILLUMINANT1_DAYLIGHT, MUINT8)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_CALIBRATION_TRANSFORM1)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_CALIBRATION_TRANSFORM2)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_COLOR_TRANSFORM1)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_COLOR_TRANSFORM2)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_FORWARD_MATRIX1)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
    CONFIG_METADATA_END()
    //==========================================================================
    CONFIG_METADATA_BEGIN(MTK_SENSOR_FORWARD_MATRIX2)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(0, 1), MRational)
         CONFIG_ENTRY_VALUE(MRational(1, 1), MRational)
    CONFIG_METADATA_END()
//------------------------------------------------------------------------------
STATIC_METADATA_END()


