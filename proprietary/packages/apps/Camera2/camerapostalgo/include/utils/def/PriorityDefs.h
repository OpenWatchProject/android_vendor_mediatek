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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DEF_PRIORITYDEFS_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DEF_PRIORITYDEFS_H_
/******************************************************************************
 *  Priority Definitions.
 ******************************************************************************/
#include <utils/ThreadDefs.h> // for SCHED_OTHER

/******************************************************************************
 *  Nice value (SCHED_OTHER)
 ******************************************************************************/
enum
{
    //
    NICE_CAMERA_PASS1               = ANDROID_PRIORITY_FOREGROUND,
    NICE_CAMERA_PASS2               = ANDROID_PRIORITY_FOREGROUND,
    NICE_CAMERA_VSS_PASS2           = ANDROID_PRIORITY_NORMAL,
    NICE_CAMERA_SM_PASS2            = ANDROID_PRIORITY_NORMAL - 8,
    NICE_CAMERA_JPEG                = ANDROID_PRIORITY_NORMAL,
    NICE_CAMERA_SHOTCB              = ANDROID_PRIORITY_NORMAL,
    //
    NICE_CAMERA_CAPTURE             = ANDROID_PRIORITY_NORMAL,
    //
    NICE_CAMERA_SHUTTER_CB          = ANDROID_PRIORITY_NORMAL,
    NICE_CAMERA_ZIP_IMAGE_CB        = ANDROID_PRIORITY_NORMAL,
    NICE_CAMERA_FRAMEWORK_CB        = ANDROID_PRIORITY_NORMAL,
    //
    //
    NICE_CAMERA_EIS                 = ANDROID_PRIORITY_NORMAL,
    NICE_CAMERA_VHDR                = ANDROID_PRIORITY_NORMAL,
    //Lomo Jni for Matrix Menu of Effect
    NICE_CAMERA_LOMO                = ANDROID_PRIORITY_NORMAL-8,
    //
    //  3A-related
    NICE_CAMERA_3A_MAIN             = (ANDROID_PRIORITY_NORMAL - 7),
    NICE_CAMERA_AE                  = (ANDROID_PRIORITY_NORMAL - 8),
    NICE_CAMERA_AF                  = (ANDROID_PRIORITY_NORMAL - 20),
    NICE_CAMERA_FLK                 = (ANDROID_PRIORITY_NORMAL - 3),
    NICE_CAMERA_TSF                 = (ANDROID_PRIORITY_NORMAL - 8),
    NICE_CAMERA_STT_AF              = (ANDROID_PRIORITY_NORMAL - 20),
    NICE_CAMERA_STT                 = (ANDROID_PRIORITY_NORMAL - 8),
    NICE_CAMERA_CCU                 = (ANDROID_PRIORITY_NORMAL - 4),
    NICE_CAMERA_AE_Start            = (ANDROID_PRIORITY_NORMAL - 7),
    NICE_CAMERA_AF_Start            = (ANDROID_PRIORITY_NORMAL - 7),
    NICE_CAMERA_ISP_BPCI            = (ANDROID_PRIORITY_NORMAL - 7),
    NICE_CAMERA_CONFIG_STTPIPE      = (ANDROID_PRIORITY_NORMAL - 7),
    NICE_CAMERA_ResultPool          = (ANDROID_PRIORITY_NORMAL - 4),
    //
    // Pipeline-related
    NICE_CAMERA_PIPELINE_P1NODE     = (ANDROID_PRIORITY_NORMAL - 4),
    //
    NICE_CAMERA_P1_ENQUE            = (ANDROID_PRIORITY_NORMAL - 20),
    NICE_CAMERA_P1_DEQUE            = (ANDROID_PRIORITY_NORMAL - 16),

    NICE_CAMERA_PIPEMGR_BASE        = ANDROID_PRIORITY_FOREGROUND,
};


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DEF_PRIORITYDEFS_H_

