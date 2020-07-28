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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_STREAM_STREAMID_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_STREAM_STREAMID_H_
//
#include <stdint.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/******************************************************************************
 * HALv1 streamId table
 *      value          external/internal of pipeline        description
 *     0 ~ 2^31-1               internal                   app's streams
 *   2^31 ~ 2^63-1              internal                   hal's streams
 ******************************************************************************/

/******************************************************************************
 * HALv3 streamId table
 *      value          external/internal of pipeline        description
 *        -1                    external                     NO_STREAM
 *     0 ~ 2^31-1               external                    fwk/client id
 *       2^31                   external                 application's control
 *  2^31+1 ~ 2^31+2^28-1        external                  physical camera id
 *     2^31+2^28                external                 last partial metadata
 * 2^31+2^28+1 ~ 2^32-1         external                preserved for future use
 *   2^32 ~ 2^63-1              internal                    hal's streams
 ******************************************************************************/

enum : int64_t
{
    eSTREAMID_NO_STREAM             = -1L,
    //==========================================================================
    //==========================================================================
    eSTREAMID_BEGIN_OF_EXTERNAL     = (0x00000L),
    //==========================================================================
        eSTREAMID_BEGIN_OF_APP      = eSTREAMID_BEGIN_OF_EXTERNAL,
        eSTREAMID_BEGIN_OF_FWK      = eSTREAMID_BEGIN_OF_EXTERNAL,
        //======================================================================
        // ...
        //======================================================================
        eSTREAMID_END_OF_APP        = (0x80000000L),
        eSTREAMID_END_OF_FWK        = eSTREAMID_END_OF_APP,
        //=======================================================================
        // ...
        eSTREAMID_BEGIN_OF_PHYSIC_ID= (0x80000001L),
        // ...
        eSTREAMID_END_OF_PHYSIC_ID  = (0x8FFFFFFFL),
        //=======================================================================
        eSTREAMID_LAST_METADATA     = (0x90000000L),
    //==========================================================================
    eSTREAMID_END_OF_EXTERNAL       = (0xFFFFFFFFL),
    //==========================================================================
    //==========================================================================
    eSTREAMID_BEGIN_OF_INTERNAL     = (0x100000000L),
    //==========================================================================

    //==========================================================================
    eSTREAMID_END_OF_INTERNAL       = (0x7FFFFFFFFFFFFFFFL),
    //==========================================================================
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_STREAM_STREAMID_H_

