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

namespace dbg_cam_reservec_param_0 {
/******************************************************************************
 *
 ******************************************************************************/

//ReserveC Parameter Structure
typedef enum
{
    RESERVEC_TAG_VERSION = 0,
    /* add tags here */

    RESERVEC_TAG_END
}DEBUG_RESERVEC_TAG_T;

// TEST_C debug info
enum { RESERVEC_DEBUG_TAG_VERSION = 0 };
enum { RESERVEC_DEBUG_NON_TAG_VAL_SIZE = 10000 };
enum { RESERVEC_DEBUG_TAG_SIZE = (RESERVEC_TAG_END+RESERVEC_DEBUG_NON_TAG_VAL_SIZE) };

//gmv
enum { MF_MAX_FRAME = 8 };
enum {MF_GMV_DEBUG_TAG_GMV_X
    , MF_GMV_DEBUG_TAG_GMV_Y
    , MF_GMV_DEBUG_TAG_ITEM_SIZE
    };
enum { MF_GMV_DEBUG_TAG_SIZE = (MF_GMV_DEBUG_TAG_ITEM_SIZE) };

//eis
enum { MF_EIS_DEBUG_TAG_WINDOW = 32 };
enum {MF_EIS_DEBUG_TAG_MV_X
    , MF_EIS_DEBUG_TAG_MV_Y
    , MF_EIS_DEBUG_TAG_TRUST_X
    , MF_EIS_DEBUG_TAG_TRUST_Y
    , MF_EIS_DEBUG_TAG_ITEM_SIZE
    };
enum { MF_EIS_DEBUG_TAG_SIZE = (MF_EIS_DEBUG_TAG_WINDOW*MF_EIS_DEBUG_TAG_ITEM_SIZE) };

struct DEBUG_RESERVEC_INFO_T {
    uint32_t Tag[RESERVEC_DEBUG_TAG_SIZE];
    const uint32_t count;
    const uint32_t gmvCount;
    const uint32_t eisCount;
    const uint32_t gmvSize;
    const uint32_t eisSize;
    int32_t  gmvData[MF_MAX_FRAME][MF_GMV_DEBUG_TAG_ITEM_SIZE];
    uint32_t eisData[MF_MAX_FRAME][MF_EIS_DEBUG_TAG_WINDOW][MF_EIS_DEBUG_TAG_ITEM_SIZE];

    DEBUG_RESERVEC_INFO_T() : count(2)  //gmvCount + eisCount
                            //
                            , gmvCount(MF_MAX_FRAME)
                            , eisCount(MF_MAX_FRAME)
                            //
                            , gmvSize(MF_GMV_DEBUG_TAG_SIZE)
                            , eisSize(MF_EIS_DEBUG_TAG_SIZE)
    {
    }
};

struct DEBUG_RESERVEC_INFO_S
{
    debug_exif_field Tag[RESERVEC_DEBUG_TAG_SIZE];
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace

