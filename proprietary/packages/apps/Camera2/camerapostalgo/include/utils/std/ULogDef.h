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

#ifndef __MTKCAM_UTILS_STD_ULOGDEF_H__
#define __MTKCAM_UTILS_STD_ULOGDEF_H__

namespace NSCam {
namespace Utils {
namespace ULog {


typedef unsigned int ModuleId;
typedef unsigned int RequestSerial;

static constexpr unsigned int ULOG_LAYER_MASK    = 0xfff00000;
static constexpr unsigned int ULOG_GROUP_MASK    = 0x000ff000;
static constexpr unsigned int ULOG_RESERVED_MASK = 0x00000800;

inline constexpr ModuleId MODULE_ID(unsigned int layerBits, unsigned int groupBits, unsigned int reserved, unsigned int moduleSerial)
{
    return static_cast<ModuleId>((layerBits << 20) | (groupBits << 12) | (reserved << 11) | moduleSerial);
}


// Module will be logged only if (moduleId & filter & 0xfff00000)) != 0 && (moduleId & filter & 0x000ff0000) != 0
// Either layerBits or filterBits must NOT be zero
enum ModuleIdEnum
{
    /*
     * NOTE: You MUST also fill the module name in ULogTable.cpp
     */
    MOD_INVALID                 = 0, // Never print any log

    // -- HAL & Camera device
    MOD_HAL                     = MODULE_ID(0x001, 0x01, 0, 0),
    MOD_CAMERA_HAL_SERVER,
    MOD_CAMDEV3_IMP             = MODULE_ID(0x001, 0x07, 0, 1),
    MOD_APP_STREAM_MGR,
    MOD_CAMERA_DEVICE           = MODULE_ID(0x001, 0x87, 0, 1), // HAL3 interface only

    // -- Pipeline model
    MOD_PIPELINE_CAPTURE_GROUP  = MODULE_ID(0x002, 0x02, 0, 0),
    MOD_PIPELINE_PRERELEASE,
    MOD_PIPELINE_MODEL_CAPTURE,
    MOD_PIPELINE_MODEL          = MODULE_ID(0x002, 0x07, 0, 1),
    MOD_DEFAULT_PIPELINE_MODEL,
    MOD_PIPELINE_POLICY,
    MOD_FEATURE_SETTING_POLICY,

    // -- Hw Node
    MOD_HW_NODE                 = MODULE_ID(0x004, 0x01, 0, 0),
    MOD_P1_NODE                 = MODULE_ID(0x004, 0x01, 0, 1), // Basic/P1/3A filterBits = 0x1
    MOD_P1_NODE_2,
    MOD_P1_NODE_3,
    MOD_P2_CAP_NODE             = MODULE_ID(0x004, 0x02, 0, 1), // Capture filterBits = 0x2
    MOD_JPEG_NODE,
    MOD_RAW16_NODE,
    MOD_RAW16_NODE_2,
    MOD_JPS_NODE,
    MOD_P2_STR_NODE             = MODULE_ID(0x004, 0x04, 0, 1), // Streaming filterBits = 0x4
    MOD_FD_NODE,
    MOD_PDE_NODE,
    MOD_BASE_NODE               = MODULE_ID(0x004, 0x06, 0, 1),
    MOD_P2N_COMMON,

    // -- P2 processors, feature pipe
    MOD_3A_FRAMEWORK            = MODULE_ID(0x008, 0x01, 0, 0),
    MOD_ISP_MGR,
    MOD_P2_CAP_PROC             = MODULE_ID(0x008, 0x02, 0, 1),
    MOD_FPIPE_CAPTURE,
    MOD_P2_STR_PROC             = MODULE_ID(0x008, 0x04, 0, 1),
    MOD_P2_PROC_COMMON          = MODULE_ID(0x008, 0x06, 0, 1),
    MOD_P2_BASIC_PROC,
    MOD_FPIPE_COMMON,

    // -- Feature pipe nodes
    MOD_CAPTURE_FPNODE_BEGIN    = MODULE_ID(0x010, 0x02, 0, 0),
    MOD_CAPTURE_ROOT,
    MOD_CAPTURE_RAW,
    MOD_CAPTURE_BSS,
    MOD_CAPTURE_P2A,
    MOD_CAPTURE_MULTIFRAME,
    MOD_CAPTURE_DEPTH,
    MOD_CAPTURE_BOKEH,
    MOD_CAPTURE_FUSION,
    MOD_CAPTURE_MDP,
    MOD_CAPTURE_FD,
    MOD_CAPTURE_YUV,
    MOD_CAPTURE_YUV_1,
    MOD_CAPTURE_YUV_2,
    MOD_CAPTURE_YUV_3,

    MOD_STREAMING_FPNODE_BEGIN  = MODULE_ID(0x010, 0x04, 0, 0),
    MOD_FPIPE_STREAMING,
    MOD_STREAMING_ROOT,
    MOD_STREAMING_P2A,
    MOD_STREAMING_P2NR,
    MOD_STREAMING_VNR,
    MOD_STREAMING_P2A_MDP,
    MOD_STREAMING_RSC,
    MOD_STREAMING_DEPTH,
    MOD_STREAMING_BOKEH,
    MOD_STREAMING_VENDOR_MDP,
    MOD_STREAMING_FOV,
    MOD_STREAMING_FOV_FEFM,
    MOD_STREAMING_FOV_WARP,
    MOD_STREAMING_EIS,
    MOD_STREAMING_EIS_WARP,
    MOD_STREAMING_MDP,
    MOD_STREAMING_HELPER,
    MOD_STREAMING_VENDOR        = MODULE_ID(0x010, 0x84, 0, 0), // group 0x80 for 3rd-party
    MOD_STREAMING_TPI,
    MOD_STREAMING_TPI_DISP,
    MOD_STREAMING_TPI_ASYNC,
    MOD_STREAMING_TPI_PLUGIN,
    MOD_FPNODE_UNKNOWN          = MODULE_ID(0x010, 0xff, 0, 0),
    MOD_FPNODE_SEQ_QUEUE,

    // -- Sub feature pipe & its nodes
    MOD_SUB_FPIPE               = MODULE_ID(0x020, 0x01, 0, 0),
    MOD_SFPIPE_DEPTH            = MODULE_ID(0x020, 0x06, 0, 1), // Both capture & streaming
    MOD_SFPIPE_DEPTH_P2A,
    MOD_SFPIPE_DEPTH_P2A_BAYER,
    MOD_SFPIPE_DEPTH_N3D,
    MOD_SFPIPE_DEPTH_NR3D,
    MOD_SFPIPE_DEPTH_GF,
    MOD_SFPIPE_DEPTH_FD,
    MOD_SFPIPE_DEPTH_WPE,
    MOD_SFPIPE_DEPTH_DPE,
    MOD_SFPIPE_DEPTH_OCC,
    MOD_SFPIPE_DEPTH_HW_OCC,
    MOD_SFPIPE_DEPTH_WMF,
    MOD_SFPIPE_DEPTH_DLDEPTH,

    // -- Library & algorithm adapters
    MOD_BASIC_LIBRARY_BEGIN     = MODULE_ID(0x080, 0x01, 0, 0),
    MOD_PD_HAL,
    MOD_LMV_HAL,
    MOD_VHDR_HAL,
    MOD_CAPTURE_LIBRARY_BEGIN   = MODULE_ID(0x080, 0x02, 0, 0),
    MOD_LIB_MFNR,
    MOD_LIB_AINR,
    MOD_LIB_SWNR,
    MOD_LIB_HDR,
    MOD_ABF_ADAPT,
    MOD_STREAMING_LIBRARY_BEGIN = MODULE_ID(0x080, 0x04, 0, 0),
    MOD_EIS_HAL,
    MOD_3DNR_HAL,
    MOD_FSC_HAL,
    MOD_RSS_HAL,
    MOD_FEATURE_LIB_COMMON      = MODULE_ID(0x080, 0x06, 0, 1),

    // -- Algorithm
    MOD_ALGORITHM               = MODULE_ID(0x100, 0x01, 0, 0),

    // -- Driver
    MOD_DRV_NORMAL_PIPE         = MODULE_ID(0x200, 0x01, 0, 1),
    MOD_DRV_SENSOR,
    MOD_DRV_CAMCAL,
    MOD_DRV_DIP                 = MODULE_ID(0x200, 0x06, 0, 1),
    MOD_DRV_RSC,
    MOD_DRV_FD,
    MOD_DRV_DPE,
    MOD_DRV_WPE,
    MOD_DRV_OWE,
    MOD_DRV_MFB,
    MOD_DRV_MSS,
    MOD_DRV_MSF,
    MOD_DRV_LPCNR,
    MOD_DRV_COMMON              = MODULE_ID(0x200, 0x07, 0, 1),
    MOD_IOPIPE_EVENT            = MODULE_ID(0x200, 0x10, 0, 1),

    // -- Utilities
    MOD_UTILITY                 = MODULE_ID(0x400, 0x07, 0, 0), // Shared for all small utilities
    MOD_IMAGE_BUFFER,
    MOD_UNIFIED_LOG,

    MOD_END
};


enum RequestTypeId
{
    /*
     * NOTE: You MUST also fill the RequestType name in ULogTable.cpp
     */
    REQ_INVALID_ID = 0,
    REQ_APP_REQUEST,
    REQ_PIPELINE_FRAME,
    REQ_P2_CAP_REQUEST,
    REQ_P2_STR_REQUEST,
    REQ_CAP_FPIPE_REQUEST,
    REQ_STR_FPIPE_REQUEST,
    REQ_DEPTH_REQUEST,
    REQ_QBUF_INFO,
    REQ_QPARAMS,

    REQUEST_TYPE_END
};


}
}
}

#endif

