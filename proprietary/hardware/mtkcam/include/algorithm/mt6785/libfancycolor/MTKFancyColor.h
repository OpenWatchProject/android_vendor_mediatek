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

#ifndef _MTK_FANCY_COLOR_H
#define _MTK_FANCY_COLOR_H

#include "MTKFancyColorType.h"
#include "MTKFancyColorErrCode.h"

//#define SEGMENT_CMODEL              (1)

typedef enum DRVFancyColorObject_s {
    DRV_FANCY_COLOR_OBJ_NONE = 0,
    DRV_FANCY_COLOR_OBJ_SW,
    DRV_FANCY_COLOR_OBJ_UNKNOWN = 0xFF,
} DrvFancyColorObject_e;

typedef enum FANCY_COLOR_STATE_ENUM
{
    FANCY_COLOR_STATE_STANDBY=0,            // After Create Obj or Reset
    FANCY_COLOR_STATE_INIT,                 // After Called Init
    FANCY_COLOR_STATE_PROCESS,              // After Called Main
    FANCY_COLOR_STATE_PROCESS_DONE,         // After Finish Main
} FANCY_COLOR_STATE_ENUM;

typedef enum
{
    FANCY_COLOR_FEATURE_BEGIN,              // minimum of feature id
    FANCY_COLOR_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    FANCY_COLOR_FEATURE_SET_WORKBUF_INFO,   // feature id to set working buffer address
    FANCY_COLOR_FEATURE_SET_PROC_INFO,      // feature id to set image info
    FANCY_COLOR_FEATURE_GET_RESULT,         // feature id to get result
    FANCY_COLOR_FEATURE_MAX                 // maximum of feature id
}   FANCY_COLOR_FEATURE_ENUM;


typedef enum
{
    FANCY_COLOR_EFFECT_NORMAL,

    FANCY_COLOR_EFFECT_STROKE,
    FANCY_COLOR_EFFECT_SILHOUETTE, //Reserved. SILHOUETTE is not implemented
    FANCY_COLOR_EFFECT_RADIAL_BLUR,

    FANCY_COLOR_EFFECT_TYPE_NUM,
} FANCY_COLOR_EFFECT_ENUM;

//////////////////////////////////////////


typedef struct
{
    MINT32 papercut_width;
    MUINT8 fg_alpha_threshold;
    MFLOAT radiar_blur_boundary;

} FANCY_CORE_TUNING_PARA_STRUCT, *P_FANCY_CORE_TUNING_PARA_STRUCT;

typedef struct
{
    //unsigned char mem_alignment;

    //unsigned char img_orientation;

    //unsigned int  debug_level;

    // input images and resolution
    unsigned int  input_color_img_width;
    unsigned int  input_color_img_height;
    unsigned int  input_color_img_stride;
    unsigned char *input_color_img_addr;

    unsigned int  input_alpha_mask_width;
    unsigned int  input_alpha_mask_height;
    unsigned int  input_alpha_mask_stride;
    unsigned char *input_alpha_mask_addr;

    // output resolution
    /*
    unsigned int  output_mask_width;
    unsigned int  output_mask_height;
    unsigned int  output_mask_stride;

    unsigned int  output_img_width;
    unsigned int  output_img_height;
    unsigned int  output_img_stride;*/

    //center point
    MINT32 center_x;
    MINT32 center_y;

    // input face info

    FANCY_CORE_TUNING_PARA_STRUCT     tuning_para;
} FANCY_CORE_SET_ENV_INFO_STRUCT, *P_FANCY_CORE_SET_ENV_INFO_STRUCT;

typedef struct
{
    unsigned int ext_mem_size;
    unsigned char *ext_mem_start_addr; // working buffer start address
} FANCY_CORE_SET_WORK_BUF_INFO_STRUCT, *P_FANCY_CORE_SET_WORK_BUF_INFO_STRUCT ;
typedef struct
{
    MINT32 left;
    MINT32 top;
    MINT32 right;
    MINT32 bottom;

} FANCY_RECT, *P_FANCY_RECT;
typedef struct
{

    FANCY_COLOR_EFFECT_ENUM color_effect;
    FANCY_RECT rect_in;

} FANCY_CORE_SET_PROC_INFO_STRUCT, *P_FANCY_CORE_SET_PROC_INFO_STRUCT;


typedef struct
{
    MUINT8 *output_mask_addr;
    MUINT8 *output_img_addr;

    FANCY_RECT rect_out;

} FANCY_CORE_RESULT_STRUCT, *P_FANCY_CORE_RESULT_STRUCT;


/*******************************************************************************
*
********************************************************************************/
class MTKFancyColor {
public:
    static MTKFancyColor* createInstance(DrvFancyColorObject_e eobject);
    virtual void   destroyInstance(MTKFancyColor* obj) = 0;

    virtual ~MTKFancyColor(){}
    // Process Control
    virtual MRESULT Init(void *InitInData, void *InitOutData);
    virtual MRESULT Main(void);
    virtual MRESULT Reset(void);

    // Feature Control
    virtual MRESULT FeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:


};

class AppFancyColorTmp : public MTKFancyColor {
public:
    //
    static MTKFancyColor* getInstance();
    virtual void destroyInstance(){};
    //
    AppFancyColorTmp() {};
    virtual ~AppFancyColorTmp() {};
};

#endif

