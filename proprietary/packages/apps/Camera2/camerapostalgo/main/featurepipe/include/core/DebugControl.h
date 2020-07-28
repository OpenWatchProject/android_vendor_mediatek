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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_DEBUG_CONTROL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_DEBUG_CONTROL_H_

// Properties
#define KEY_TUNING_BUF_POOL_PROTECT     "vendor.debug.fpipe.tuning.protect"

#define VAL_TUNING_BUF_PROTECT          (-1) // -1:not set, 0: force no use, 1: force use

// include files
#define TRACE_BUFFER_POOL               0
#define TRACE_CAM_GRAPH                 0
#define TRACE_CAM_NODE                  0
#define TRACE_CAM_PIPE                  0
#define TRACE_CAM_THREAD_NODE           0
#define TRACE_COUNT_DOWN_LATCH          0
#define TRACE_THREAD_SAFE_QUEUE         0
#define TRACE_WAIT_QUEUE                0

// src files
#define TRACE_CAM_THREAD                0
#define TRACE_DEBUG_UTIL                0
#define TRACE_FAT_IMAGE_BUFFER_POOL     0
#define TRACE_GRAPHIC_BUFFER_POOL       0
#define TRACE_IIBUFFER                  0
#define TRACE_IMAGE_BUFFER_POOL         0
#define TRACE_TUNING_BUFFER_POOL        0
#define TRACE_NATIVEBUFFER_WRAPPER      0
#define TRACE_PROFILE                   0
#define TRACE_SHARED_BUFFER_POOL        0
#define TRACE_SHARED_BUFFER_POOL_QUOTA  0
#define TRACE_STRING_UTIL               0
#define TRACE_SYNC_UTIL                 0
#define TRACE_SEQ_UTIL                  0
#define TRACE_IO_UTIL                   0
#define TRACE_WAIT_HUB                  0
#define TRACE_JPEG_ENCODE_THREAD        0

#define TRACE_CAPTURE_FEATURE_PIPE      0
#define TRACE_CAPTURE_FEATURE_COMMON    0
#define TRACE_CAPTURE_FEATURE_DATA      1
#define TRACE_CAPTURE_FEATURE_NODE      0
#define TRACE_YUV_NODE                  1
#define TRACE_ROOT_NODE                 1
#define TRACE_MULTIFRAME_NODE           1


#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_DEBUG_CONTROL_H_

// only FeaturePipe/core files include this DebugControl.h
// so always redefine PIPE_MODULE_TAG back to FeaturePipe
#undef PIPE_MODULE_TAG
#define PIPE_MODULE_TAG "PostAlgo/FeaturePipe"
