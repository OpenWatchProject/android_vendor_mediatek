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

#ifndef _MTK_IMAGE_SEGMENT_H
#define _MTK_IMAGE_SEGMENT_H

#include "MTKImageSegmentType.h"
#include "MTKImageSegmentErrCode.h"

typedef enum DRVImageSegmentObject_s {
    DRV_IMAGE_SEGMENT_OBJ_NONE = 0,
    DRV_IMAGE_SEGMENT_OBJ_SW,
    DRV_IMAGE_SEGMENT_OBJ_UNKNOWN = 0xFF,
} DrvImageSegmentObject_e;

typedef enum IMAGE_SEGMENT_STATE_ENUM
{
    IMAGE_SEGMENT_STATE_STANDBY=0,            // After Create Obj or Reset
    IMAGE_SEGMENT_STATE_INIT,                 // After Called Init
    IMAGE_SEGMENT_STATE_PROCESS,              // After Called Main
    IMAGE_SEGMENT_STATE_PROCESS_DONE,         // After Finish Main
} IMAGE_SEGMENT_STATE_ENUM;

typedef enum
{
    IMAGE_SEGMENT_FEATURE_BEGIN,              // minimum of feature id
    IMAGE_SEGMENT_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    IMAGE_SEGMENT_FEATURE_GET_SAVE_WORKBUF_SIZE,
    IMAGE_SEGMENT_FEATURE_SET_WORKBUF_INFO,   // feature id to set working buffer address
    IMAGE_SEGMENT_FEATURE_SET_PROC_INFO,      // feature id to set image info
    IMAGE_SEGMENT_FEATURE_GET_RESULT,         // feature id to get result
    IMAGE_SEGMENT_FEATURE_SET_SAVE_INFO,         // feature id to set the save buffer address
    IMAGE_SEGMENT_FEATURE_SET_SAVE_WORKBUF_ADDR,         // feature id to set the save buffer address
    IMAGE_SEGMENT_FEATURE_MAX                 // maximum of feature id
}   IMAGE_SEGMENT_FEATURE_ENUM;

typedef enum
{
    SEGMENT_IMAGE_ROTATION_0,
    SEGMENT_IMAGE_ROTATION_90_CW,
    SEGMENT_IMAGE_ROTATION_90_CCW,
    SEGMENT_IMAGE_ROTATION_180
} SEGMENT_IMAGE_ROTATION_ENUM;

typedef enum
{
    SEGMENT_SCENARIO_AUTO,
    SEGMENT_SCENARIO_SELECTION,
    SEGMENT_SCENARIO_SCRIBBLE_FG,
    SEGMENT_SCENARIO_SCRIBBLE_BG,
    SEGMENT_SCENARIO_SAVE,
    SEGMENT_SCENARIO_NUM
} SEGMENT_SCENARIO_ENUM;

typedef enum
{
    SEGMENT_OBJECT,
    SEGMENT_FOREGROUND
} SEGMENT_MODE_ENUM;

// trimap values
typedef enum
{
    SEGMENT_TRIMAP_BACKGROUND   = 0,
    SEGMENT_TRIMAP_FOREGROUND   = 255,
    SEGMENT_TRIMAP_UNKNOWN      = 128
} SEGMENT_TRIMAP_TYPE_ENUM;

//////////////////////////////////////////

typedef struct
{
    int x;
    int y;
} Point;

typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
} Rect;

typedef struct
{
    unsigned char alpha_mode;
    float         seg_ratio;
} SEGMENT_CORE_TUNING_PARA_STRUCT, *P_SEGMENT_CORE_TUNING_PARA_STRUCT;

typedef struct
{
    unsigned char mem_alignment;

    unsigned char img_orientation;

    unsigned int  debug_level;

    // input images and resolution
    unsigned int  input_color_img_width;
    unsigned int  input_color_img_height;
    unsigned int  input_color_img_stride;
    unsigned char *input_color_img_addr;

    unsigned int  input_depth_img_width;
    unsigned int  input_depth_img_height;
    unsigned int  input_depth_img_stride;
    unsigned char *input_depth_img_addr;

    unsigned int  input_occ_img_width;
    unsigned int  input_occ_img_height;
    unsigned int  input_occ_img_stride;
    unsigned char *input_occ_img_addr;

    unsigned int  input_scribble_width;
    unsigned int  input_scribble_height;
    unsigned int  input_scribble_stride;

    // output resolution (normal operation)
    unsigned int  output_mask_width;
    unsigned int  output_mask_height;
    unsigned int  output_mask_stride;

    // output resolution (save)
    unsigned int  save_width;
    unsigned int  save_height;
    unsigned int  save_color_img_stride;
    unsigned int  save_mask_stride;

    // input face info
    unsigned int  face_num;
    Rect          *face_pos;
    int           *face_rip;

    unsigned int  working_buffer_size;
    unsigned int  save_working_buffer_size;

    SEGMENT_CORE_TUNING_PARA_STRUCT     tuning_para;
} SEGMENT_CORE_SET_ENV_INFO_STRUCT, *P_SEGMENT_CORE_SET_ENV_INFO_STRUCT;

typedef struct
{
    unsigned int ext_mem_size;
    unsigned char *ext_mem_start_addr; // working buffer start address
} SEGMENT_CORE_SET_WORK_BUF_INFO_STRUCT, *P_SEGMENT_CORE_SET_WORK_BUF_INFO_STRUCT ;

typedef struct
{
    // output resolution (save)
    unsigned int  save_width;
    unsigned int  save_height;
    unsigned int  save_color_img_stride;
    unsigned int  save_mask_stride;
} SEGMENT_CORE_SET_SAVE_INFO_STRUCT, *P_SEGMENT_CORE_SET_SAVE_INFO_STRUCT ;

typedef struct
{
    unsigned char scenario;
    unsigned char mode;
    unsigned char undo;
    union
    {
        unsigned char *input_scribble_addr;
        unsigned char *input_color_img_addr;
    };
    unsigned char *prev_output_mask_addr;
    unsigned char *working_buffer_addr;
    Rect          input_user_roi;
} SEGMENT_CORE_SET_PROC_INFO_STRUCT, *P_SEGMENT_CORE_SET_PROC_INFO_STRUCT;

typedef struct
{
    unsigned char *output_mask_addr;
    Point         center;
    Rect          bbox;
} SEGMENT_CORE_RESULT_STRUCT, *P_SEGMENT_CORE_RESULT_STRUCT;

/*******************************************************************************
*
********************************************************************************/
class MTKImageSegment {
public:
    static MTKImageSegment* createInstance(DrvImageSegmentObject_e eobject);
    virtual void   destroyInstance(MTKImageSegment* obj) = 0;

    virtual ~MTKImageSegment(){}
    // Process Control
    virtual MRESULT Init(void *InitInData, void *InitOutData);
    virtual MRESULT Main(void);
    virtual MRESULT Reset(void);

    // Feature Control
    virtual MRESULT FeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:
};

class AppImageSegmentTmp : public MTKImageSegment {
public:
    //
    static MTKImageSegment* getInstance();
    virtual void destroyInstance(){};
    //
    AppImageSegmentTmp() {};
    virtual ~AppImageSegmentTmp() {};
};

#endif