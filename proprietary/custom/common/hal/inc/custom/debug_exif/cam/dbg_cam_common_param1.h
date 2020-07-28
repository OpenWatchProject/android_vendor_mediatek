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

namespace dbg_cam_common_param_1 {
/******************************************************************************
 *
 ******************************************************************************/

// Common debug info
enum { CMN_DEBUG_TAG_VERSION = 1 };
enum { CMN_DEBUG_TAG_SUBVERSION = 2 };
#define CMN_DEBUG_TAG_VERSION_DP ((CMN_DEBUG_TAG_SUBVERSION << 16) | CMN_DEBUG_TAG_VERSION)

//Common Parameter Structure
typedef enum
{
    //BEGIN_OF_EXIF_TAG
    CMN_TAG_VERSION = 0,
    CMN_TAG_SHOT_MODE,
    CMN_TAG_CAM_MODE,
    CMN_TAG_PIPELINE_UNIQUE_KEY,
    CMN_TAG_PIPELINE_FRAME_NUMBER,
    CMN_TAG_PIPELINE_REQUEST_NUMBER,
    CMN_TAG_DOWNSCALE_DENOISE_THRES,
    CMN_TAG_DOWNSCALE_DENOISE_RATIO,
    CMN_TAG_DOWNSCALE_DENOISE_WIDTH,
    CMN_TAG_DOWNSCALE_DENOISE_HIGHT,
    CMN_TAG_SWNR_THRESHOLD,
    CMN_TAG_SYS_FREE_MEMORY_MBYTES,
    CMN_DEBUG_TAG_SIZE   //END_OF_EXIF_TAG
}DEBUG_CMN_TAG_T;

typedef struct DEBUG_CMN_INFO_S
{
    debug_exif_field Tag[CMN_DEBUG_TAG_SIZE];
} DEBUG_CMN_INFO_T;


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace

