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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTKCAM_HWNODE_P2_DEBUG_CONTROL_H_
#define _MTKCAM_HWNODE_P2_DEBUG_CONTROL_H_

#define NEW_CODE                    1
#define REPLACE_HAL1_P2F            1
#define USE_BASIC_PROCESSOR         0
#define USE_CAPTURE_PROCESSOR       0

#define P2_POLICY_DYNAMIC           0
#define P2_POLICY_FORCE_BASIC       1
#define P2_POLICY_FORCE_STREAMING   2

#define P2_DUMP_PATH "/sdcard/p2"

#define KEY_P2_PROCESSOR            "debug.mtkcam.p2.processor"
#define KEY_P2_DUMP                 "debug.mtkcam.p2.dump"
#define KEY_P2_DUMP_COUNT           "debug.mtkcam.p2.dump.count"
#define KEY_P2_DUMP_PATH            "debug.mtkcam.p2.dump.path"
#define KEY_P2_LOG                  "debug.mtkcam.p2.log"
#define KEY_P2_REDIRECT             "debug.mtkcam.p2.redirect"
#define KEY_P2_SCANLINE             "debug.mtkcam.p2.scanline"
#define KEY_TRACE_P2                "trace.p2"

#define VAL_P2_PROCESSOR            P2_POLICY_DYNAMIC
#define VAL_P2_BASIC                1
#define VAL_P2_STREAMING            2

#define VAL_P2_DUMP                 0
#define VAL_P2_DUMP_COUNT           1
#define VAL_P2_DUMP_PATH            P2_DUMP_PATH
#define VAL_P2_LOG                  0
#define VAL_P2_REDIRECT             0
#define VAL_P2_SCANLINE             0

#define USE_CLASS_TRACE             1

#define TRACE_BASIC_PROCESSOR       0
#define TRACE_BASIC_HIGH_SPEED      0
#define TRACE_CAPTURE_PROCESSOR     0
#define TRACE_CROPPER               0
#define TRACE_DISPATCH_PROCESSOR    0
#define TRACE_LMV_INFO              0
#define TRACE_MW_FRAME              0
#define TRACE_MW_FRAME_REQUEST      0
#define TRACE_MW_IMG                0
#define TRACE_MW_INFO               0
#define TRACE_MW_META               0
#define TRACE_PARTIAL_P2_REQUEST    0
#define TRACE_P2_DUMP_PLUGIN        0
#define TRACE_P2_FRAME_HOLDER       0
#define TRACE_P2_FRAME_REQUEST      0
#define TRACE_P2_INFO               0
#define TRACE_P2_LOGGER             0
#define TRACE_P2_NODE               0
#define TRACE_P2_REQUEST            0
#define TRACE_P2_SCANLINE_PLUGIN    0
#define TRACE_P2_UTIL               0
#define TRACE_PROCESSOR             0
#define TRACE_STREAMING_PROCESSOR   0
#define TRACE_STREAMING_3DNR        0
#define TRACE_STREAMING_EIS         0

#endif // _MTKCAM_HWNODE_P2_DEBUG_CONTROL_H_

#undef P2_MODULE_TAG
#define P2_MODULE_TAG   "MtkCam/P2"
#undef P2_CLASS_TAG
#undef P2_TRACE
