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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file fsc_defs.h
*
* FSC Defs Header File
*/


#ifndef _FSC_DEFS_H_
#define _FSC_DEFS_H_

#include <mtkcam/def/BuiltinTypes.h>

namespace NSCam{
namespace FSC{

/*********************************************************************
* Define Value
*********************************************************************/
#define EFSC_FSC_ENABLED(x)         ((x) & (EFSC_MODE_MASK_FSC_EN))
#define EFSC_SUBPIXEL_ENABLED(x)    ((x) & (EFSC_MODE_MASK_SUBPIXEL_EN))
#define EFSC_DEBUG_ENABLED(x)       ((x) & (EFSC_MODE_MASK_DEBUG_LEVEL))

#define FSC_CROP_INT(x)             ((x) >> 8)
#define FSC_MAX_RSSO_WIDTH          320
#define FSC_MAX_SCALING_RATIO       1000000
#define FSC_MACRO_INF_RATIO_UNIT    10000
#define FSC_DAC_DAMPING_TIME_TO_US  1000
#define FSC_CROPPING_GROUP_NUM      2

#define FSC_DEBUG_ENABLE_PROPERTY               "vendor.debug.fsc.debug.enable"
#define FSC_SUBPIXEL_ENABLE_PROPERTY            "vendor.debug.fsc.subpixel.enable"
#define FSC_FORCE_MAX_CROP_PROPERTY             "vendor.debug.fsc.force.max_crop"
#define FSC_MAX_CROP_VALUE_PROPERTY             "vendor.debug.fsc.max_crop.value"

/*********************************************************************
* ENUM
*********************************************************************/

/**
*@brief Return enum of FSC Mode for P2 flow.
*/
enum EFSC_MODE_MASK
{
    EFSC_MODE_MASK_FSC_NONE          = 0,       // disable fsc
    EFSC_MODE_MASK_FSC_EN            = 1 << 0,  // enable fsc
    EFSC_MODE_MASK_SUBPIXEL_EN       = 1 << 1,  // enable subpixel
    EFSC_MODE_MASK_DEBUG_LEVEL       = 1 << 7,  // debug
};

/*********************************************************************
* Struct
*********************************************************************/
struct FSC_CROPPING_GROUP_STRUCT
{
    MINT32    groupID;
    MRectF    resultF;

    FSC_CROPPING_GROUP_STRUCT() : groupID(0)
    {
    }
};

struct FSC_CROPPING_RESULT_STRUCT
{
    FSC_CROPPING_GROUP_STRUCT croppingGroup[FSC_CROPPING_GROUP_NUM];
};

}; // namespace FSC
}; // namespace NSCam
#endif // _FSC_DEFS_H_

