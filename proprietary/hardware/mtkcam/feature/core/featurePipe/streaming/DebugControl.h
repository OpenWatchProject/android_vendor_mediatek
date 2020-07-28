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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_DEBUG_CONTROL_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_DEBUG_CONTROL_H_

#define KEY_FORCE_EIS                 "vendor.debug.fpipe.force.eis"
#define KEY_FORCE_EIS_25              "vendor.debug.fpipe.force.eis25"
#define KEY_FORCE_EIS_30              "vendor.debug.fpipe.force.eis30"
#define KEY_FORCE_EIS_QUEUE           "vendor.debug.fpipe.force.eisq"
#define KEY_FORCE_VFB                 "vendor.debug.fpipe.force.vfb"
#define KEY_FORCE_VFB_EX              "vendor.debug.fpipe.force.vfb.ex"
#define KEY_FORCE_3DNR                "vendor.debug.fpipe.force.3dnr"
#define KEY_FORCE_VHDR                "vendor.debug.fpipe.force.vhdr"
#define KEY_FORCE_WARP_PASS           "vendor.debug.fpipe.force.warp.pass"
#define KEY_FORCE_GPU_OUT             "vendor.debug.fpipe.force.gpu.out"
#define KEY_FORCE_GPU_RGBA            "vendor.debug.fpipe.force.gpu.rgba"
#define KEY_FORCE_IMG3O               "vendor.debug.fpipe.force.img3o"
#define KEY_FORCE_BUF                 "vendor.debug.fpipe.force.buf"
#define KEY_FORCE_VENDOR              "vendor.debug.fpipe.force.vendor"
#define KEY_ENABLE_VENDOR             "vendor.debug.fpipe.enable.vendor"
#define KEY_ENABLE_VENDOR_CUS_SIZE    "vendor.debug.fpipe.vendor.cus.size"
#define KEY_ENABLE_VENDOR_INPLACE     "vendor.debug.fpipe.vendor.inplace"
#define KEY_ENABLE_VENDOR_CUS_FORMAT  "vendor.debug.fpipe.vendor.cus.format"
#define KEY_FORCE_VENDOR_FOV          "vendor.debug.fpipe.force.vendor.fov"
#define KEY_ENABLE_VENDOR_FOV         "vendor.debug.fpipe.enable.vendor.fov"
#define KEY_USE_PER_FRAME_SETTING     "vendor.debug.fpipe.frame.setting"
#define KEY_DEBUG_DUMP                "vendor.debug.fpipe.force.dump"
#define KEY_DEBUG_DUMP_COUNT          "vendor.debug.fpipe.force.dump.count"
#define KEY_DEBUG_DUMP_BY_RECORDNO    "vendor.debug.fpipe.dump.by.recordno"
#define KEY_FORCE_RSC_TUNING          "vendor.debug.fpipe.force.rsc.tuning"
#define KEY_FORCE_PRINT_IO            "vendor.debug.fpipe.force.printio"


#define STR_ALLOCATE                  "pool allocate:"

#define USE_YUY2_FULL_IMG             1
#define USE_WPE_STAND_ALONE           0

#define SUPPORT_GPU_YV12          1
#define SUPPORT_GPU_CROP          1
#define SUPPORT_VFB               0
#define SUPPORT_VENDOR_NODE       0
#define SUPPORT_VENDOR_FOV_NODE   0
#define SUPPORT_FAKE_EIS_30       0
#define SUPPORT_FAKE_WPE          0
#if (MTKCAM_FOV_USE_WPE == 1)
#define SUPPORT_WPE               1
#else
#define SUPPORT_WPE               0
#endif

#define SUPPORT_VENDOR_CUS_SIZE     0
#define SUPPORT_VENDOR_INPLACE      0
#define SUPPORT_VENDOR_CUS_FORMAT   0
#define SUPPORT_VENDOR_FULL_FORMAT  eImgFmt_NV21

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#define SUPPORT_FOV               1
#else
#define SUPPORT_FOV               0
#endif

#define DEBUG_TIMER               1

#define DEV_VFB_READY             0
#define DEV_P2B_READY             0

#define NO_FORCE  0
#define FORCE_ON  1
#define FORCE_OFF 2

#define VAL_FORCE_EIS               0
#define VAL_FORCE_EIS_25            0
#define VAL_FORCE_EIS_30            0
#define VAL_FORCE_EIS_QUEUE         0
#define VAL_FORCE_VFB               0
#define VAL_FORCE_VFB_EX            0
#define VAL_FORCE_3DNR              0
#define VAL_FORCE_VHDR              0
#define VAL_FORCE_WARP_PASS         0
#define VAL_FORCE_GPU_OUT           0
#define VAL_FORCE_GPU_RGBA          (!SUPPORT_GPU_YV12)
#define VAL_FORCE_IMG3O             0
#define VAL_FORCE_BUF               0
#define VAL_FORCE_VENDOR            0
#define VAL_FORCE_VENDOR_FOV        0
#define VAL_DEBUG_DUMP              0
#define VAL_DEBUG_DUMP_COUNT        1
#define VAL_DEBUG_DUMP_BY_RECORDNO  0
#define VAL_USE_PER_FRAME_SETTING   0
#define VAL_FORCE_RSC_TUNING        0
#define VAL_FORCE_PRINT_IO          0

#define TRACE_STREAMING_FEATURE_COMMON  0
#define TRACE_STREAMING_FEATURE_DATA    0
#define TRACE_STREAMING_FEATURE_NODE    0
#define TRACE_STREAMING_FEATURE_PIPE    0
#define TRACE_STREAMING_FEATURE_USAGE   0
#define TRACE_STREAMING_FEATURE_TIMER   0
#define TRACE_IMG_BUFFER_STORE          0
#define TRACE_ROOT_NODE                 0
#define TRACE_P2_CAM_CONTEXT            0
#define TRACE_P2A_NODE                  0
#define TRACE_P2A_3DNR                  0
#define TRACE_P2A_VHDR                  0
#define TRACE_FM_HAL                    0
#define TRACE_EIS_NODE                  0
#define TRACE_GPU_NODE                  0
#define TRACE_MDP_NODE                  0
#define TRACE_VFB_NODE                  0
#define TRACE_FD_NODE                   0
#define TRACE_P2B_NODE                  0
#define TRACE_VENDOR_NODE               0
#define TRACE_VMDP_NODE                 0
#define TRACE_RSC_NODE                  0
#define TRACE_WARP_NODE                 0
#define TRACE_HELPER_NODE               0
#define TRACE_QPARAMS_BASE              0
#define TRACE_WARP_BASE                 0
#define TRACE_GPU_WARP                  0
#define TRACE_WPE_WARP                  0
#define TRACE_COOKIE_STORE              0
#define TRACE_NORMAL_STREAM_BASE        0
#define TRACE_RSC_STREAM_BASE           0
#define TRACE_RSC_TUNING_STREAM         0
#define TRACE_WARP_STREAM_BASE          0
#define TRACE_GPU_WARP_STREAM_BASE      0
#define TRACE_WPE_WARP_STREAM_BASE      0
#define TRACE_MDP_WRAPPER               0
// dual zoom
#define TRACE_FOV_NODE                  0
#define TRACE_FOVWARP_NODE              0
#define TRACE_FOV_HAL                   0
#define TRACE_P2A_FOV                   0
#define TRACE_VENDOR_FOV_NODE           0
// dual preview mode
#define TRACE_N3D_NODE                  0
#define TRACE_N3DP2_NODE                0
#define TRACE_EIS_Q_CONTROL             0

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_DEBUG_CONTROL_H_

#ifdef PIPE_MODULE_TAG
#undef PIPE_MODULE_TAG
#endif
#define PIPE_MODULE_TAG "MtkCam/StreamingPipe"

