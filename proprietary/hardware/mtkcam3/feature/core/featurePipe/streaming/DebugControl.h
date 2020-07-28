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
#define KEY_FORCE_EIS_30              "vendor.debug.fpipe.force.eis30"
#define KEY_FORCE_EIS_QUEUE           "vendor.debug.fpipe.force.eisq"
#define KEY_FORCE_3DNR                "vendor.debug.fpipe.force.3dnr"
#define KEY_FORCE_DSDN20              "vendor.debug.fpipe.force.dsdn20"
#define KEY_FORCE_DSDN25              "vendor.debug.fpipe.force.dsdn20"
#define KEY_FORCE_VHDR                "vendor.debug.fpipe.force.vhdr"
#define KEY_FORCE_WARP_PASS           "vendor.debug.fpipe.force.warp.pass"
#define KEY_FORCE_GPU_OUT             "vendor.debug.fpipe.force.gpu.out"
#define KEY_FORCE_GPU_RGBA            "vendor.debug.fpipe.force.gpu.rgba"
#define KEY_FORCE_IMG3O               "vendor.debug.fpipe.force.img3o"
#define KEY_FORCE_BUF                 "vendor.debug.fpipe.force.buf"
#define KEY_FORCE_DUMMY               "vendor.debug.fpipe.force.dummy"
#define KEY_FORCE_TPI_YUV             "vendor.debug.fpipe.force.tpi.yuv"
#define KEY_FORCE_TPI_ASYNC           "vendor.debug.fpipe.force.tpi.async"
#define KEY_ENABLE_DUMMY              "vendor.debug.fpipe.enable.dummy"
#define KEY_ENABLE_PURE_YUV           "vendor.debug.fpipe.enable.pureyuv"
#define KEY_USE_PER_FRAME_SETTING     "vendor.debug.fpipe.frame.setting"
#define KEY_DEBUG_DUMP                "vendor.debug.fpipe.force.dump"
#define KEY_DEBUG_DUMP_COUNT          "vendor.debug.fpipe.force.dump.count"
#define KEY_DEBUG_DUMP_BY_RECORDNO    "vendor.debug.fpipe.dump.by.recordno"
#define KEY_FORCE_RSC_TUNING          "vendor.debug.fpipe.force.rsc.tuning"
#define KEY_FORCE_PRINT_IO            "vendor.debug.fpipe.force.printio"

#define KEY_DEBUG_TPI                 "vendor.debug.tpi.s"
#define KEY_DEBUG_TPI_LOG             "vendor.debug.tpi.s.log"
#define KEY_DEBUG_TPI_DUMP            "vendor.debug.tpi.s.dump"
#define KEY_DEBUG_TPI_ERASE           "vendor.debug.tpi.s.erase"
#define KEY_DEBUG_TPI_SCAN            "vendor.debug.tpi.s.scan"
#define KEY_DEBUG_TPI_BYPASS          "vendor.debug.tpi.s.bypass"



#define STR_ALLOCATE                  "pool allocate:"

#define MAX_TPI_COUNT                 ((MUINT32)3)
#define DCESO_DELAY_COUNT             ((MUINT32)2)

#define BUF_ALLOC_ALIGNMENT_BIT       ((MUINT32)6) //64 align

#define TPI_RECORD_QUEUE_DELAY_CNT    ((MUINT32)18)

#define USE_YUY2_FULL_IMG             1
#define USE_NV21_FULL_IMG             1
#define USE_WPE_STAND_ALONE           0

#define SUPPORT_3A_HAL            1
#define SUPPORT_ISP_HAL           1
#define SUPPORT_GRAPHIC_BUFFER    1
#define SUPPORT_GPU_YV12          1
#define SUPPORT_GPU_CROP          1
#define SUPPORT_VENDOR_NODE       0
#define SUPPORT_DUMMY_NODE        0
#define SUPPORT_PURE_YUV          0
#define SUPPORT_FAKE_EIS_30       0
#define SUPPORT_FAKE_WPE          0
#if (MTKCAM_FOV_USE_WPE == 1)
#define SUPPORT_WPE               1
#else
#define SUPPORT_WPE               0
#endif

#define SUPPORT_P2AMDP_THREAD_MERGE   0

#define SUPPORT_VENDOR_SIZE           0
#define SUPPORT_VENDOR_FORMAT         0
#define SUPPORT_VENDOR_FULL_FORMAT    eImgFmt_NV21

#define DEBUG_TIMER               1

#define DEV_VFB_READY             0
#define DEV_P2B_READY             0

#define NO_FORCE  0
#define FORCE_ON  1
#define FORCE_OFF 2

#define VAL_FORCE_EIS               0
#define VAL_FORCE_EIS_30            0
#define VAL_FORCE_EIS_QUEUE         0
#define VAL_FORCE_3DNR              0
#define VAL_FORCE_DSDN20            0
#define VAL_FORCE_VHDR              0
#define VAL_FORCE_WARP_PASS         0
#define VAL_FORCE_GPU_OUT           0
#define VAL_FORCE_GPU_RGBA          (!SUPPORT_GPU_YV12)
#define VAL_FORCE_IMG3O             0
#define VAL_FORCE_BUF               0
#define VAL_FORCE_DUMMY             0
#define VAL_FORCE_TPI_YUV           0
#define VAL_FORCE_TPI_ASYNC         0
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
#define TRACE_DSDNCFG                   0
#define TRACE_ROOT_NODE                 0
#define TRACE_TOF_NODE                  0
#define TRACE_P2_CAM_CONTEXT            0
#define TRACE_P2A_NODE                  0
#define TRACE_P2A_3DNR                  0
#define TRACE_P2SM_NODE                 0
#define TRACE_P2NR_NODE                 0
#define TRACE_VNR_NODE                  0
#define TRACE_EIS_NODE                  0
#define TRACE_GPU_NODE                  0
#define TRACE_VENDOR_NODE               0
#define TRACE_VMDP_NODE                 0
#define TRACE_DUMMY_NODE                0
#define TRACE_NULL_NODE                 0
#define TRACE_RSC_NODE                  0
#define TRACE_WARP_NODE                 0
#define TRACE_HELPER_NODE               0
#define TRACE_QPARAMS_BASE              0
#define TRACE_WARP_BASE                 0
#define TRACE_GPU_WARP                  0
#define TRACE_WPE_WARP                  0
#define TRACE_COOKIE_STORE              0
#define TRACE_NORMAL_STREAM_BASE        0
#define TRACE_P2DIP_STREAM_BASE         0
#define TRACE_RSC_STREAM_BASE           0
#define TRACE_RSC_TUNING_STREAM         0
#define TRACE_WARP_STREAM_BASE          0
#define TRACE_GPU_WARP_STREAM_BASE      0
#define TRACE_WPE_WARP_STREAM_BASE      0
#define TRACE_MDP_WRAPPER               0
#define TRACE_VNR_HAL                   0
// dual zoom
#define TRACE_FOV_NODE                  0
#define TRACE_FOVWARP_NODE              0
#define TRACE_FOV_HAL                   0
#define TRACE_P2A_FOV                   0
// dual preview mode
#define TRACE_N3D_NODE                  0
#define TRACE_N3DP2_NODE                0
#define TRACE_EIS_Q_CONTROL             0
#define TRACE_TPI_NODE                  0
#define TRACE_TPI_ASYNC_NODE            0
#define TRACE_TPI_USAGE                 0
#define TRACE_SFP_DEPTH_NODE            0
#define TRACE_SFP_BOKEH_NODE            0


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

