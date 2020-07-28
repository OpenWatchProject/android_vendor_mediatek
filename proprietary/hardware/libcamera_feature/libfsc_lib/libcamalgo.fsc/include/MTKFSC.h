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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MTK_FSC_H
#define _MTK_FSC_H

#include "MTKFSCType.h"
#include "MTKFSCErrCode.h"

typedef struct
{
    MINT32 macro_to_inf_ratio; //Macro-to-Infinity content scale ratio Golden calibrated  Varies with camera model

    MINT32 max_cropped_ratio; // Maximum avalible proportion of image to be cropped (In width of sensor size)

    // "Lens VCM DAC values. Focused on infinity and macro focus. Per-module calibrated (EEPROM→NVRAM)"
    MINT32 dac_Inf;
    MINT32 dac_Macro;

    MINT32 af_table_start; //dac_table_min;
    MINT32 af_table_end; //dac_table_max;

    MINT32 readout_time_mus; // Rolling shutter delay between the start of exposing the 1-st row and the start of exposing the last row.
    MINT32 af_time_mus; //Time from AF start to AF stable
} FSC_INIT_STRUCT, *P_FSC_INIT_STRUCT;


typedef struct
{
    MINT32 max_cropped_ratio;
} FSC_DAC_TO_SCALE_STRUCT, *P_FSC_DAC_TO_SCALE_STRUCT;

typedef struct
{
    MINT32 request_frame_num; //The serial of the frame which is affected by a certain AF update
    MINT32 rolling_shutter_percentage; //The rolling shutter percentage of the frame, whose serial is "requestFrameNum", at which the AF starts to update
    MINT32 target_DAC; //The target DAC value of a certain AF update
} FSC_AF_UPDATE_STRUCT, *P_FSC_AF_UPDATE_STRUCT;


typedef struct
{
    MINT32 request_frame_num; //Requested frame serial
    MINT32 exposureTime_mus; //The exposure time of this frame

    MINT32 w_sensor_out; //Sensor output image width
    MINT32 h_sensor_out; //Sensor output image height

    //The crop settings of FSC input, relative to sensor coordinate system
    MINT32 x_sensor_crop;
    MINT32 y_sensor_crop;
    MINT32 w_sensor_crop;
    MINT32 h_sensor_crop;

    //FSC input image size
    MINT32 w_input;
    MINT32 h_input;
} FSC_QUERY_INFO_STRUCT, *P_FSC_QUERY_INFO_STRUCT;

typedef struct
{
    MINT32 x_sensor_crop;
    MINT32 y_sensor_crop;
    MINT32 w_sensor_crop;
    MINT32 h_sensor_crop;

    MINT32 x_input_crop;
    MINT32 y_input_crop;
    MINT32 w_input_crop;
    MINT32 h_input_crop;
} FSC_CROP_RESULT_INFO_STRUCT, *P_FSC_CROP_RESULT_INFO_STRUCT;

#define FSC_EIS_SLICES 18
#define FSC_WARPING_SLICE_NUM (FSC_EIS_SLICES+1)

typedef struct
{
    MINT32 scale_list[FSC_WARPING_SLICE_NUM];//Relative scale for backward interpolation to achieve FSC
} FSC_WARPING_RESULT_INFO_STRUCT, *P_FSC_WARPING_RESULT_INFO_STRUCT;


//FSC_FEATURE_GET_IMAGE_SCALE_INFO: Compute Scale for a Whole Image (Debug / C Model)
typedef struct
{
    MINT32 request_frame_num;
    MINT32 exposureTime_mus;
    MINT32 h_sensor_out;
    MINT32 y_sensor_crop;
    MINT32 h_sensor_crop;

} FSC_IMAGE_SCALE_INFO_STRUCT, *P_FSC_IMAGE_SCALE_INFO_STRUCT;

typedef struct
{
    MFLOAT image_scale;
} FSC_IMAGE_SCALE_RESULT_STRUCT, *P_FSC_IMAGE_SCALE_RESULT_STRUCT;

//FSC_FEATURE_GET_MIN_CROP_INFO:  Return Min Remaining Region after Cropping
typedef struct
{
    MINT32 w_sensor_out; //Sensor output image width
    MINT32 h_sensor_out; //Sensor output image height

    //The crop settings of FSC input, relative to sensor coordinate system
    MINT32 x_sensor_crop;
    MINT32 y_sensor_crop;
    MINT32 w_sensor_crop;
    MINT32 h_sensor_crop;

    //FSC input image size
    MINT32 w_input;
    MINT32 h_input;
} FSC_SENSOR_INFO_STRUCT, *P_FSC_SENSOR_INFO_STRUCT;

// FSC_FEATURE_GET_VERSION: Return Algorithm Version
typedef struct
{
    MINT8 rMainVer[5];
    MINT8 rSubVer[5];
    MINT8 rPatchVer[5];
} FSC_VerInfo;


typedef enum
{
    FSC_FEATURE_BEGIN = 0,

    FSC_FEATURE_SET_DAC_TO_SCALE_INFO,
    FSC_FEATURE_SET_AF_UPDATE_INFO,
    FSC_FEATURE_GET_CROP_RESULT_INFO, //FSC_FEATURE_GET_CROP_RESULT_INFO is deprecated
    FSC_FEATURE_GET_WARPING_RESULT_INFO,
    FSC_FEATURE_GET_IMAGE_SCALE_INFO,
    FSC_FEATURE_GET_MIN_CROP_INFO,
    FSC_FEATURE_GET_MAX_SCALE_INFO,
    FSC_FEATURE_GET_CROP_REMAIN_SIZE_RATIO,
    FSC_FEATURE_GET_VERSION,
    FSC_FEATURE_MAX
} FSC_FEATURE_ENUM;

/*******************************************************************************
*
********************************************************************************/
class MTKFSC {
public:
    static MTKFSC* createInstance();
    virtual void   destroyInstance() = 0;

    virtual ~MTKFSC();
    // Process Control
    virtual MRESULT FSCInit(P_FSC_INIT_STRUCT InitInData);    //InitInData : P_FSC_INIT_STRUCT

    // Feature Control
    //             FeatureID                   |         pParaIn                |            pParaOut              |
    //  FSC_FEATURE_SET_DAC_TO_SCALE_INFO      |  P_FSC_DAC_TO_SCALE_STRUCT     |              NULL                | Update the getDacScale function
    //  FSC_FEATURE_SET_AF_UPDATE_INFO         |  P_FSC_AF_UPDATE_STRUCT        |              NULL                | Every time AF updates.
    //  FSC_FEATURE_GET_WARPING_RESULT_INFO    |  P_FSC_QUERY_INFO_STRUCT       | P_FSC_WARPING_RESULT_INFO_STRUCT | Query Scale List for different image Y coordinates
    //  FSC_FEATURE_GET_IMAGE_SCALE_INFO       |  P_FSC_IMAGE_SCALE_INFO_STRUCT | P_FSC_IMAGE_SCALE_RESULT_STRUCT  | Compute Scale for a Whole Image (Debug / C Model)
    //  FSC_FEATURE_GET_MIN_CROP_INFO          |  P_FSC_SENSOR_INFO_STRUCT      | P_FSC_CROP_RESULT_INFO_STRUCT    | Return Min Remaining Region after Cropping
    //  FSC_FEATURE_GET_MAX_SCALE_INFO         |          NULL                  | P_FSC_IMAGE_SCALE_RESULT_STRUCT  | Return Max Relative Scale (AF Table End to AF Table Start) (Debug / C Model)
    //  FSC_FEATURE_GET_CROP_REMAIN_SIZE_RATIO |  P_FSC_IMAGE_SCALE_INFO_STRUCT | P_FSC_IMAGE_SCALE_RESULT_STRUCT  | Return Remaining Image Size Ratio after Cropping (for Whole-Image Scaling)
    virtual MRESULT FSCFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

#endif

